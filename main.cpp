// ================================================================
//  AIUB Campus Final – Complete Scene with Seasons & FIREWORKS
//  CSC 3224 – Computer Graphics Project
//
//  Features:
//    • Sun with glow and rays
//    • Brick footpath above the road
//    • Road with dashed centre line and kerb
//    • Football field (right side) with markings
//    • Basketball court (middle) with lane and three‑point arc
//    • Pavement / concrete path (left side)
//    • Optimised roadside trees (palm, oak, blossom) and lamp posts
//    • Left background buildings (5 grey towers)
//    • Right background buildings (Annex-9 bg + C-Building bg)
//    • Annex-9 foreground building
//    • C-Building (spherical globe, 6 floors with glass/metallic bands)
//    • D-Building (main AIUB academic building, 10 floors, lounges)
//    • SPRING MODE ('s') – falling petals, cherry blossoms
//    • AUTUMN MODE ('a') – falling leaves      [Emad]
//    • WINTER MODE ('w') – falling snow
//    • RAINY MODE ('r') – raindrops, umbrellas, gray sky, wet ground
//    • SUMMER MODE ('g') – mangoes, jackfruits, heat haze, stall, birds
//    • FIREWORKS MODE ('f') – spectacular fireworks display!
//    • RAIN SOUND + SPEED – 'r' starts rain w/ sound,
//                            'p' speeds rain up, 'o' slows it down  [Sadia]
//    • THUNDERSTORM ('t') – cycles intensity 1→2→3→off
//                           jagged lightning + flash + sky bolts    [Sadia]
//
//  Dynamic transformations (Emad):
//    • TRANSLATION : car speed via ',' (slower) and '.' (faster)
//    • SCALING     : autumn leaf size via 'k' (smaller) and 'l' (larger)
//                    — applied with glScalef() in drawAutumn()
//    • ROTATION    : autumn leaves spin as they fall — per-leaf angle
//                    auto-advances each tick, applied via glRotatef()

//  Draw order (painter's algorithm, back → front):
//    1. Sky
//    2. Sun
//    3. Ground
//    4. Road & sports fields
//    5. Brick footpath
//    6. Roadside trees & lamp posts
//    7. Left background buildings
//    8. Right background buildings
//    9. Annex-9
//   10. C-Building
//   11. D-Building
//   12. Fireworks (drawn on top)
//
//  Coordinate space : -1.0 to +1.0 (X and Y)
//  Window size      : 1400 x 800
// ================================================================
// ================================================================
// Cross-platform OpenGL / GLUT includes.
//   • Windows  → use freeglut + windows.h + mmsystem.h for MCI audio
//   • macOS    → use the framework header (no windows.h, no MCI)
// ================================================================
#ifdef _WIN32
#include <windows.h>
#include <GL/freeglut.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
#include <GLUT/glut.h> // macOS framework
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>  // sprintf, printf, etc.
#include <string.h> // strcat, strlen, etc.

#define PI 3.14159265f
#define MAX_FALLING 85
#define MAX_RESTING 55
#define MAX_RAIN 200

// ── Cloud system for spring season ─────────────────────────────────
#define MAX_CLOUDS 8
static float cloudX[MAX_CLOUDS];
static float cloudY[MAX_CLOUDS];
static float cloudSize[MAX_CLOUDS];
static float cloudSpeed[MAX_CLOUDS];
static bool cloudActive[MAX_CLOUDS];
// ── Season mode flags ─────────────────────────────────────────────
// ── Autumn season state (Emad) ────────────────────────────────────
// Autumn = falling leaves drifting down
// each leaf moves slowly
#define MAX_LEAVES 95
static bool autumnMode = false;     // toggled by pressing 'a'
static float leafX[MAX_LEAVES];     // X position of each leaf
static float leafY[MAX_LEAVES];     // Y position of each leaf
static float leafSpeed[MAX_LEAVES]; // how fast it falls
static int leafColor[MAX_LEAVES];   // 0=orange, 1=yellow, 2=brown, 3=red

// ── Dynamic transformation state (Emad) ───────────────────────────
// These globals back the three required transformations:
//   • carSpeedMult — translation scalar for road traffic.
//                    Adjusted at runtime via ',' (slower) and '.' (faster).
//   • leafScale    — scaling factor applied to every falling leaf via
//                    glScalef().  Adjusted via 'k' (smaller) and 'l' (larger).
//   • leafAngle[]  — per-leaf rotation angle in degrees, applied via
//                    glRotatef().  Auto-increments every tick by leafSpin[].
//   • leafSpin[]   — per-leaf spin speed (deg/tick), randomised once at
//                    init so each leaf rotates at its own pace + direction.
static float carSpeedMult = 1.0f;   // 1.0 = default traffic speed
static float leafScale = 1.0f;      // 1.0 = default leaf size
static float leafAngle[MAX_LEAVES]; // current rotation of each leaf (deg)
static float leafSpin[MAX_LEAVES];  // per-leaf rotation speed (deg/tick)

// Dynamic transformation state (Shajmin)
// playerSpeedMult — translation scalar for sports players. Applied inside updatePlayers() to every basketball and football player.
// snowScale — scaling factor applied to every snowflake in winter mode via glScalef().
// parkingArmAngle — 0 = closed (horizontal), ~85 = open
static float playerSpeedMult = 1.0f;  // 1.0 = default player speed
static float snowScale = 1.0f; // 1.0 = default snowflake size
static float parkingArmAngle = 0.0f;  // current arm angle (deg)
static float parkingArmTarget = 0.0f; // target arm angle (deg)

// Winter season state (Shajmin)
#define MAX_SNOW 200
static bool winterMode = false; // toggled by pressing 'w'
static float snowX[MAX_SNOW]; // X position of each snowflake
static float snowY[MAX_SNOW]; // Y position of each snowflake
static float snowSpeed[MAX_SNOW]; // how fast it falls

static bool summerMode = false; // toggled by pressing 'g'

// Moving sports players (Shajmin)
// 4 basketball players + 6 football players. Each one has its own
// position (X,Y) and velocity (VX,VY). On every tick, position +=
// velocity. When a player hits a wall, that velocity is reversed.
#define BBALL_COUNT 4
#define SOCCER_COUNT 6

static float bballX[BBALL_COUNT]; // basketball player X
static float bballY[BBALL_COUNT]; // basketball player Y
static float bballVX[BBALL_COUNT]; // basketball player X velocity
static float bballVY[BBALL_COUNT]; // basketball player Y velocity

static float soccerX[SOCCER_COUNT]; // football player X
static float soccerY[SOCCER_COUNT]; // football player Y
static float soccerVX[SOCCER_COUNT]; // football player X velocity
static float soccerVY[SOCCER_COUNT]; // football player Y velocity

static bool springMode = true;   // true = spring (petals), false = normal
static bool rainMode = false;    // true = rainy season, false = dry
static int rainSoundPlaying = 0; // [Sadia] 1 while rain WAV is looping

// ── Rain particles (Sadia) ────────────────────────────────────────
static float rainX[MAX_RAIN];
static float rainY[MAX_RAIN];
static float rainSpeed[MAX_RAIN];
static float rainSpeedMult = 1.0f; // [Sadia] adjustable via 'p' / 'o'

// ── Wind animation state (used only by tree sway / petal physics) ──
static float windAngle = 0.0f;   // current sway angle (degrees)
static float windDir = 1.0f;     // +1 or -1
static float windSway = 0.0f;    // current sway in degrees (-8 to +8)
static float windSwayMax = 8.0f; // max sway degrees
// ── [A] Global cycle state ────────────────────────────────────────
static float dayAngle = 0.28f;  // 0 → PI : sun travels left → right
static float nightAngle = 0.0f; // 0 → PI : moon arc
static int isDay = 1;           // 1 = day, 0 = night
static float breezeSpeed = 0.016f;
static float cloudScale = 1.0f;     // 1.0 = default size, +/- keys adjust this
static float cloudSpeedMult = 1.0f; // 1.0 = default, adjustable at runtime

// ── Thunderstorm state (Sadia) ────────────────────────────────────
// Triggered by pressing 't'.  Each 't' press steps intensity:
//   1st press: intensity=1 (small bolt)
//   2nd press: intensity=2 (bigger, +1 branch)
//   3rd press: intensity=3 (biggest, +3 branches)
//   4th press: turn off
// The bolt is drawn with glScalef so it appears to "grow" from the
// sky each time it strikes — another use of dynamic scaling.
static bool thunderMode = false;
static int thunderIntensity = 1;
static float thunderTimer = 0.0f;
static float thunderFlashAlpha = 0.0f;
static bool lightningVisible = false;
static float lightningTimer = 0.0f;

// Thunder scaling animation (the "grow then shrink" effect on each strike)
static float thunderScale = 0.0f;
static float thunderScaleDir = 1.0f;
static float thunderScaleMax = 0.0f;
static bool thunderScaling = false;

#define MAX_LIGHTNING_SEGS 16
static float lgSegX[MAX_LIGHTNING_SEGS + 1];
static float lgSegY[MAX_LIGHTNING_SEGS + 1];
static int lgSegCount = 0;

// Branch bolts (only at intensity 2 and 3)
#define MAX_BRANCHES 3
static float branchX[MAX_BRANCHES][9];
static float branchY[MAX_BRANCHES][9];
static int branchCount = 0;

// ── [FIREWORKS SYSTEM] ─────────────────────────────────────────────
// Firework tunables
#define FW_MAX_ROCKETS 24      // max simultaneous rockets in flight
#define FW_MAX_PARTICLES 4200  // total particle pool (all bursts combined)
#define FW_SOUND_DURATION 5.0f // seconds — matches fireWorkSoundV3.m4a length

// Rocket types
enum FWRocketType
{
    FW_CHRYSANTHEMUM = 0, // dense full sphere — classic gold
    FW_WILLOW,            // drooping long trails
    FW_RING,              // perfect ring / double ring
    FW_COMET,             // tight fast burst with long comet tail
    FW_CROSSETTE,         // splits into 4 sub-shells mid-air
    FW_STROBE,            // flashing white sparkle cloud
    FW_PEONY,             // large sphere, petals fade outward
    FW_TYPE_COUNT
};

// Launch sites (world coordinates)
struct FWLaunchSite
{
    float x, y;
    int preferType; // -1 = random
};

static const FWLaunchSite FW_SITES[] = {
    // D-Building rooftop — centre and flanks
    {0.22f, 0.530f, FW_CHRYSANTHEMUM},
    {-0.02f, 0.535f, FW_WILLOW},
    {0.48f, 0.528f, FW_PEONY},
    // C-Building antenna tip
    {-0.715f, 0.580f, FW_RING},
    // Annex-9 roof
    {0.16f, 0.100f, FW_COMET},
    // Left background buildings (tall ones)
    {-0.94f, 0.490f, FW_CROSSETTE},
    {-0.68f, 0.375f, FW_STROBE},
    {-0.82f, 0.440f, FW_CHRYSANTHEMUM},
    // Right background area
    {0.88f, 0.280f, FW_WILLOW},
    // Top-row lamp posts as footpath launchers
    {-0.38f, -0.640f, FW_COMET},
    {0.34f, -0.640f, FW_RING},
    {0.84f, -0.640f, FW_PEONY},
    // Bottom-row lamp posts
    {-0.67f, -0.920f, FW_STROBE},
    {0.59f, -0.920f, FW_CROSSETTE},
};
static const int FW_SITE_COUNT = (int)(sizeof(FW_SITES) / sizeof(FW_SITES[0]));

// Per-particle state
struct FWParticle
{
    float x, y;   // world position
    float vx, vy; // velocity
    float ax, ay; // acceleration (gravity + drag)
    float life;   // remaining life 0..1
    float decay;  // life lost per tick
    float size;   // render radius
    unsigned char r, g, b;
    unsigned char trailR, trailG, trailB;
    bool active;
    bool isTrail;    // true = launch smoke trail
    bool isSpark;    // tiny secondary sparkle
    int strobePhase; // for FW_STROBE flicker
};

// Per-rocket state
struct FWRocket
{
    float x, y;   // current position
    float tx, ty; // target burst position
    float vx, vy; // ascent velocity
    float life;   // 0..1  (1=just launched, 0=burst)
    bool active;
    bool hasBurst;
    FWRocketType type;
    // colour of this rocket's burst
    unsigned char r, g, b;
    unsigned char r2, g2, b2; // secondary colour (ring / double)
    int siteIdx;              // which launch site
    float trailTimer;         // smoke-puff spawn timer
};

// Global firework state
static bool fwActive = false;      // master on/off
static float fwTimer = 0.0f;       // seconds elapsed since 'f' pressed
static float fwLaunchTimer = 0.0f; // countdown to next rocket launch
static int fwLaunchIdx = 0;        // next launch site index
static int fwSoundPlaying = 0;

static FWRocket fwRockets[FW_MAX_ROCKETS];
static FWParticle fwParticles[FW_MAX_PARTICLES];

// Screen flash for fireworks
static float fwFlashAlpha = 0.0f; // 0..1, decays each frame

// Tiny LCG for firework randomness (separate from petalSeed)
static unsigned int fwSeed = 0xDEADBEEFu;
static float fwRand()
{
    fwSeed = fwSeed * 1664525u + 1013904223u;
    return (float)(fwSeed & 0x7FFFu) / 32767.0f;
}
static float fwRandSym() { return fwRand() * 2.0f - 1.0f; } // -1..+1

// Colour palette (7 themed palettes, one per rocket type)
static void fwPickColour(FWRocketType type,
                         unsigned char &r, unsigned char &g, unsigned char &b,
                         unsigned char &r2, unsigned char &g2, unsigned char &b2)
{
    // Each type gets a distinct gorgeous palette
    switch (type)
    {
    case FW_CHRYSANTHEMUM:
        // Gold / amber cascade
        r = 255;
        g = 200;
        b = 40;
        r2 = 255;
        g2 = 140;
        b2 = 10;
        break;
    case FW_WILLOW:
        // Emerald green with silver tips
        r = 80;
        g = 240;
        b = 100;
        r2 = 220;
        g2 = 255;
        b2 = 220;
        break;
    case FW_RING:
        // Cyan / electric blue double ring
        r = 30;
        g = 220;
        b = 255;
        r2 = 160;
        g2 = 60;
        b2 = 255;
        break;
    case FW_COMET:
        // Crimson / hot white core
        r = 255;
        g = 60;
        b = 40;
        r2 = 255;
        g2 = 240;
        b2 = 200;
        break;
    case FW_CROSSETTE:
        // Violet / magenta split
        r = 220;
        g = 50;
        b = 220;
        r2 = 255;
        g2 = 130;
        b2 = 200;
        break;
    case FW_STROBE:
        // Pure white strobe flash
        r = 255;
        g = 255;
        b = 255;
        r2 = 200;
        g2 = 220;
        b2 = 255;
        break;
    case FW_PEONY:
        // Rose / coral large sphere
        r = 255;
        g = 80;
        b = 120;
        r2 = 255;
        g2 = 180;
        b2 = 80;
        break;
    default:
        r = 255;
        g = 255;
        b = 100;
        r2 = 255;
        g2 = 200;
        b2 = 60;
        break;
    }
    // Randomise brightness a bit
    float br = 0.75f + fwRand() * 0.25f;
    r = (unsigned char)(r * br);
    g = (unsigned char)(g * br);
    b = (unsigned char)(b * br);
}

// Spawn helpers
static FWParticle *fwAllocParticle()
{
    // Find first inactive slot — cycle through pool
    static int cursor = 0;
    for (int i = 0; i < FW_MAX_PARTICLES; i++)
    {
        int idx = (cursor + i) % FW_MAX_PARTICLES;
        if (!fwParticles[idx].active)
        {
            cursor = (idx + 1) % FW_MAX_PARTICLES;
            return &fwParticles[idx];
        }
    }
    // Pool full — evict the oldest (lowest life)
    int oldest = 0;
    float minLife = fwParticles[0].life;
    for (int i = 1; i < FW_MAX_PARTICLES; i++)
    {
        if (fwParticles[i].life < minLife)
        {
            minLife = fwParticles[i].life;
            oldest = i;
        }
    }
    return &fwParticles[oldest];
}

// Spawn one launch-trail smoke puff
static void fwSpawnTrailPuff(float x, float y,
                             unsigned char r, unsigned char g, unsigned char b)
{
    FWParticle *p = fwAllocParticle();
    p->x = x + fwRandSym() * 0.008f;
    p->y = y + fwRandSym() * 0.004f;
    p->vx = fwRandSym() * 0.0012f;
    p->vy = -0.0018f - fwRand() * 0.0025f; // drifts slightly downward (smoke)
    p->ax = 0.0f;
    p->ay = 0.0002f; // very gentle upward correction (smoke rises)
    p->life = 0.55f + fwRand() * 0.30f;
    p->decay = 0.022f + fwRand() * 0.018f;
    p->size = 0.006f + fwRand() * 0.009f;
    p->r = r;
    p->g = g;
    p->b = b;
    p->isTrail = true;
    p->isSpark = false;
    p->active = true;
    p->strobePhase = 0;
}

// Trigger screen flash
static void fwTriggerFlash()
{
    fwFlashAlpha = fminf(1.0f, fwFlashAlpha + 0.35f);
}

// Burst — emits N particles for a given firework type
static void fwBurst(float cx, float cy, FWRocketType type,
                    unsigned char r, unsigned char g, unsigned char b,
                    unsigned char r2, unsigned char g2, unsigned char b2)
{
    fwTriggerFlash(); // Flash when rocket bursts!

    float asp = 1400.0f / 800.0f; // fixed aspect for radius correction

    // Base particle count per type
    int count = 0;
    switch (type)
    {
    case FW_CHRYSANTHEMUM:
        count = 240;
        break;
    case FW_WILLOW:
        count = 180;
        break;
    case FW_RING:
        count = 120;
        break;
    case FW_COMET:
        count = 140;
        break;
    case FW_CROSSETTE:
        count = 160;
        break;
    case FW_STROBE:
        count = 200;
        break;
    case FW_PEONY:
        count = 260;
        break;
    default:
        count = 180;
        break;
    }

    for (int i = 0; i < count; i++)
    {
        FWParticle *p = fwAllocParticle();
        float angle, speed, life, decay, sz;
        bool isSpark = false;

        switch (type)
        {
        case FW_CHRYSANTHEMUM:
            angle = fwRand() * 2.0f * PI;
            speed = 0.0080f + fwRand() * 0.0100f;
            p->vx = cosf(angle) * speed / asp;
            p->vy = sinf(angle) * speed;
            p->ax = 0.0f;
            p->ay = -0.00025f;                  // Normal gravity
            life = 0.55f + fwRand() * 0.20f;    // Balanced life
            decay = 0.035f + fwRand() * 0.020f; // Moderate decay
            sz = 0.004f + fwRand() * 0.004f;
            p->r = r;
            p->g = g;
            p->b = b;
            p->trailR = (unsigned char)(r * 0.6f);
            p->trailG = (unsigned char)(g * 0.4f);
            p->trailB = 10;
            isSpark = (fwRand() < 0.15f);
            break;

        case FW_WILLOW:
            angle = fwRand() * 2.0f * PI;
            speed = 0.0060f + fwRand() * 0.0120f;
            p->vx = cosf(angle) * speed / asp;
            p->vy = sinf(angle) * speed;
            p->ax = fwRandSym() * 0.00005f;
            p->ay = -0.00060f;                  // Slightly higher gravity for willow effect
            life = 0.65f + fwRand() * 0.15f;    // Balanced life
            decay = 0.028f + fwRand() * 0.018f; // Moderate decay
            sz = 0.003f + fwRand() * 0.003f;
            p->r = r;
            p->g = g;
            p->b = b;
            p->trailR = 40;
            p->trailG = 140;
            p->trailB = 40;
            isSpark = (fwRand() < 0.10f);
            break;

        case FW_RING:
        {
            bool isOuter = (i < count / 2);
            float ringR = isOuter ? 1.0f : 0.55f;
            angle = (isOuter ? (float)i / (count / 2) : fwRand()) * 2.0f * PI;
            speed = 0.0120f * ringR;
            p->vx = cosf(angle) * speed / asp;
            p->vy = sinf(angle) * speed;
            p->ax = 0.0f;
            p->ay = -0.00015f;                  // Light gravity for ring shape
            life = 0.50f + fwRand() * 0.20f;    // Balanced life
            decay = 0.032f + fwRand() * 0.022f; // Moderate decay
            sz = 0.005f + fwRand() * 0.003f;
            if (isOuter)
            {
                p->r = r;
                p->g = g;
                p->b = b;
            }
            else
            {
                p->r = r2;
                p->g = g2;
                p->b = b2;
            }
            p->trailR = 20;
            p->trailG = 80;
            p->trailB = 160;
            isSpark = false;
            break;
        }

        case FW_COMET:
        {
            bool isHead = (fwRand() < 0.70f);
            if (isHead)
            {
                angle = fwRandSym() * 0.55f;
                speed = 0.0150f + fwRand() * 0.0080f;
                float dir = PI * 0.5f + fwRandSym() * 0.30f;
                p->vx = cosf(dir + angle) * speed / asp;
                p->vy = sinf(dir + angle) * speed;
            }
            else
            {
                angle = fwRand() * 2.0f * PI;
                speed = 0.0035f + fwRand() * 0.0060f;
                p->vx = cosf(angle) * speed / asp;
                p->vy = sinf(angle) * speed;
            }
            p->ax = 0.0f;
            p->ay = -0.00035f;
            life = 0.45f + fwRand() * 0.25f;    // Balanced life
            decay = 0.040f + fwRand() * 0.025f; // Moderate decay
            sz = isHead ? (0.005f + fwRand() * 0.006f)
                        : (0.003f + fwRand() * 0.003f);
            p->r = isHead ? r2 : r;
            p->g = isHead ? g2 : g;
            p->b = isHead ? b2 : b;
            p->trailR = 200;
            p->trailG = 50;
            p->trailB = 20;
            isSpark = (fwRand() < 0.20f);
            break;
        }

        case FW_CROSSETTE:
        {
            int group = (i * 4) / count;
            float groupAngle = group * PI * 0.5f + PI * 0.25f;
            float spread = fwRandSym() * 0.50f;
            float dist = 0.0055f + fwRand() * 0.0095f;
            angle = groupAngle + spread;
            speed = dist;
            p->vx = cosf(angle) * speed / asp;
            p->vy = sinf(angle) * speed;
            p->ax = 0.0f;
            p->ay = -0.00028f;
            life = 0.52f + fwRand() * 0.22f;    // Balanced life
            decay = 0.035f + fwRand() * 0.020f; // Moderate decay
            sz = 0.004f + fwRand() * 0.003f;
            if (group % 2 == 0)
            {
                p->r = r;
                p->g = g;
                p->b = b;
            }
            else
            {
                p->r = r2;
                p->g = g2;
                p->b = b2;
            }
            p->trailR = 180;
            p->trailG = 40;
            p->trailB = 200;
            isSpark = (fwRand() < 0.12f);
            break;
        }

        case FW_STROBE:
            angle = fwRand() * 2.0f * PI;
            speed = 0.004f + fwRand() * 0.012f;
            p->vx = cosf(angle) * speed / asp;
            p->vy = sinf(angle) * speed;
            p->ax = 0.0f;
            p->ay = -0.00020f;
            life = 0.48f + fwRand() * 0.28f;    // Balanced life
            decay = 0.038f + fwRand() * 0.022f; // Moderate decay
            sz = 0.003f + fwRand() * 0.005f;
            p->r = 255;
            p->g = 255;
            p->b = 255;
            p->trailR = 180;
            p->trailG = 200;
            p->trailB = 255;
            p->strobePhase = (int)(fwRand() * 8.0f);
            isSpark = false;
            break;

        case FW_PEONY:
            angle = fwRand() * 2.0f * PI;
            speed = 0.0050f + fabsf(fwRandSym()) * 0.0140f;
            p->vx = cosf(angle) * speed / asp;
            p->vy = sinf(angle) * speed;
            p->ax = fwRandSym() * 0.00003f;
            p->ay = -0.00022f;
            life = 0.58f + fwRand() * 0.22f;    // Balanced life
            decay = 0.030f + fwRand() * 0.018f; // Moderate decay
            sz = 0.004f + fwRand() * 0.005f;
            {
                float t = speed / 0.019f;
                p->r = (unsigned char)(r2 + (r - r2) * t);
                p->g = (unsigned char)(g2 + (g - g2) * t);
                p->b = (unsigned char)(b2 + (b - b2) * t);
            }
            p->trailR = (unsigned char)(r * 0.5f);
            p->trailG = (unsigned char)(g * 0.3f);
            p->trailB = (unsigned char)(b * 0.4f);
            isSpark = (fwRand() < 0.18f);
            break;

        default:
            angle = fwRand() * 2.0f * PI;
            speed = 0.008f;
            p->vx = cosf(angle) * speed / asp;
            p->vy = sinf(angle) * speed;
            p->ax = 0.0f;
            p->ay = -0.0003f;
            life = 0.55f;
            decay = 0.035f;
            sz = 0.004f;
            p->r = r;
            p->g = g;
            p->b = b;
            p->trailR = r;
            p->trailG = g;
            p->trailB = b;
            break;
        }

        p->x = cx + fwRandSym() * 0.004f;
        p->y = cy + fwRandSym() * 0.004f;
        p->life = life;
        p->decay = decay;
        p->size = sz;
        p->isTrail = false;
        p->isSpark = isSpark;
        p->active = true;
    }

    // Bright white central flash particles (30 tight sparks)
    for (int i = 0; i < 30; i++)
    {
        FWParticle *p = fwAllocParticle();
        float a = fwRand() * 2.0f * PI;
        float s = 0.0020f + fwRand() * 0.0050f;
        p->x = cx;
        p->y = cy;
        p->vx = cosf(a) * s / asp;
        p->vy = sinf(a) * s;
        p->ax = 0.0f;
        p->ay = -0.00015f;
        p->life = 0.35f;
        p->decay = 0.045f; // Balanced life
        p->size = 0.003f + fwRand() * 0.004f;
        p->r = 255;
        p->g = 255;
        p->b = 255;
        p->trailR = 255;
        p->trailG = 255;
        p->trailB = 200;
        p->isTrail = false;
        p->isSpark = false;
        p->active = true;
        p->strobePhase = 0;
    }
}

// Launch a rocket from a given site
static void fwLaunchRocket(int siteIdx)
{
    // Find free rocket slot
    FWRocket *rk = nullptr;
    for (int i = 0; i < FW_MAX_ROCKETS; i++)
    {
        if (!fwRockets[i].active)
        {
            rk = &fwRockets[i];
            break;
        }
    }
    if (!rk)
        return;

    const FWLaunchSite &site = FW_SITES[siteIdx % FW_SITE_COUNT];

    rk->x = site.x + fwRandSym() * 0.04f;
    rk->y = site.y;
    // Target: burst somewhere in the upper sky
    rk->tx = rk->x + fwRandSym() * 0.25f;
    rk->ty = 0.55f + fwRand() * 0.38f;

    // Calculate velocity to reach target
    float dy = rk->ty - rk->y;
    float dx = rk->tx - rk->x;
    float travelTicks = 28.0f + fwRand() * 18.0f; // ticks to reach burst
    rk->vx = dx / travelTicks;
    rk->vy = dy / travelTicks;

    rk->life = 1.0f;
    rk->hasBurst = false;
    rk->active = true;
    rk->siteIdx = siteIdx;
    rk->trailTimer = 0.0f;

    // Type
    FWRocketType t;
    if (site.preferType >= 0 && fwRand() > 0.30f)
        t = (FWRocketType)site.preferType;
    else
        t = (FWRocketType)((int)(fwRand() * FW_TYPE_COUNT));
    rk->type = t;

    fwPickColour(t, rk->r, rk->g, rk->b, rk->r2, rk->g2, rk->b2);
}

// ================================================================
//  Rain sound (Sadia) — kept cross-platform like fwPlaySound() so
//  the project still compiles on macOS as well as Windows.
// ================================================================
static void playRainSound()
{
#ifdef _WIN32
    // ---- Windows: MCI with full-path detection, then PlaySound fallback ----
    mciSendString("close rainsnd", NULL, 0, NULL);

    char fullPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, fullPath);
    strcat(fullPath, "\\rain.wav");

    char openCmd[256];
    sprintf(openCmd, "open \"%s\" type waveaudio alias rainsnd", fullPath);

    if (mciSendString(openCmd, NULL, 0, NULL) == 0)
    {
        mciSendString("play rainsnd repeat", NULL, 0, NULL); // loops forever
        rainSoundPlaying = 1;
        return;
    }

    // Fallback to PlaySound API
    PlaySound("rain.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    rainSoundPlaying = 1;

#elif __APPLE__
    // ---- macOS: use afplay inside a flag-controlled loop ----
    // The loop checks /tmp/aiub_rain_active each iteration.  Removing
    // the flag file in stopRainSound() lets the loop exit cleanly.
    // (Plain `pkill afplay` on its own would NOT stop the loop —
    //  the shell would just relaunch afplay on the next iteration.)
    system("pkill -f 'afplay rain.wav' 2>/dev/null"); // belt-and-braces cleanup
    system("touch /tmp/aiub_rain_active 2>/dev/null");
    if (system("(while [ -f /tmp/aiub_rain_active ]; do afplay rain.wav 2>/dev/null; done) >/dev/null 2>&1 &") == 0)
    {
        rainSoundPlaying = 1;
        return;
    }
    // Try one level up (build folder layout)
    if (system("(while [ -f /tmp/aiub_rain_active ]; do afplay ../rain.wav 2>/dev/null; done) >/dev/null 2>&1 &") == 0)
    {
        rainSoundPlaying = 1;
        return;
    }
#endif
}

static void stopRainSound()
{
#ifdef _WIN32
    mciSendString("stop rainsnd", NULL, 0, NULL);
    mciSendString("close rainsnd", NULL, 0, NULL);
    PlaySound(NULL, NULL, 0);
#elif __APPLE__
    // 1) Remove the flag — the while loop exits on its next check
    system("rm -f /tmp/aiub_rain_active 2>/dev/null");
    // 2) Kill any afplay currently mid-playback so silence is immediate
    system("pkill -f 'afplay rain.wav' 2>/dev/null");
#endif
    rainSoundPlaying = 0;
}

// ================================================================
//  Reset thunderstorm (Sadia, fix by Emad)
//  Centralised so every season-switch key can wipe thunder state
//  in one line.  Without this, the lightning + flash kept rendering
//  even after switching to spring / autumn / winter / summer.
// ================================================================
static void resetThunderstorm()
{
    thunderMode = false;
    lightningVisible = false;
    thunderFlashAlpha = 0.0f;
    thunderScaling = false;
    thunderScale = 0.0f;
    thunderIntensity = 1;
}

// Play sound for fireworks
static void fwPlaySound()
{
#ifdef _WIN32
    // ---- Windows: original code unchanged ----
    char fullPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, fullPath);
    strcat(fullPath, "\\fireWorkSoundV3.wav");

    mciSendString("close fwsnd", NULL, 0, NULL);

    char openCmd[256];
    sprintf(openCmd, "open \"%s\" type waveaudio alias fwsnd", fullPath);
    if (mciSendString(openCmd, NULL, 0, NULL) == 0)
    {
        mciSendString("play fwsnd", NULL, 0, NULL);
        return;
    }

    sprintf(openCmd, "open \"fireWorkSoundV3.wav\" type waveaudio alias fwsnd");
    if (mciSendString(openCmd, NULL, 0, NULL) == 0)
    {
        mciSendString("play fwsnd", NULL, 0, NULL);
        return;
    }

    if (PlaySound("fireWorkSoundV3.wav", NULL, SND_FILENAME | SND_ASYNC))
        return;
    PlaySound("C:\\Windows\\Media\\tada.wav", NULL, SND_FILENAME | SND_ASYNC);

#elif __APPLE__
    // ---- macOS: use afplay ----
    // Method 1: Try relative path (same folder as the executable)
    if (system("afplay fireWorkSoundV3.wav &") == 0)
        return;

    // Method 2: Try one level up (common when running from build folder)
    if (system("afplay ../fireWorkSoundV3.wav &") == 0)
        return;

    // Method 3: Fallback — macOS system sound
    system("afplay /System/Library/Sounds/Glass.aiff &");
#endif
}

static void fwStopSound()
{
#ifdef _WIN32
    // ---- Windows: original code unchanged ----
    mciSendString("stop fwsnd", NULL, 0, NULL);
    mciSendString("close fwsnd", NULL, 0, NULL);
    PlaySound(NULL, NULL, 0);

#elif __APPLE__
    // ---- macOS: kill any running afplay process ----
    system("pkill -x afplay 2>/dev/null");
#endif
}

// Initialize fireworks system
static void initFireworks()
{
    for (int i = 0; i < FW_MAX_ROCKETS; i++)
        fwRockets[i].active = false;
    for (int i = 0; i < FW_MAX_PARTICLES; i++)
        fwParticles[i].active = false;
    fwActive = false;
    fwTimer = 0.0f;
    fwLaunchTimer = 0.0f;
    fwLaunchIdx = 0;
    fwFlashAlpha = 0.0f;
}
// Force cleanup ALL fireworks particles and rockets
static void fwForceCleanup()
{
    // Clear all particles
    for (int i = 0; i < FW_MAX_PARTICLES; i++)
    {
        fwParticles[i].active = false;
        fwParticles[i].life = 0.0f;
    }

    // Clear all rockets
    for (int i = 0; i < FW_MAX_ROCKETS; i++)
    {
        fwRockets[i].active = false;
        fwRockets[i].hasBurst = false;
    }

    // Reset flash
    fwFlashAlpha = 0.0f;

    // Reset any lingering state
    fwActive = false;
    fwTimer = 0.0f;
    fwLaunchTimer = 0.0f;
    fwLaunchIdx = 0;
}

// Update fireworks - call every tick
static void updateFireworks(float dt)
{
    if (!fwActive)
        return;

    fwTimer += dt;

    // Wave schedule (seconds after 'f' press → sites to launch):
    static const float FW_WAVE_TIMES[] = {
        0.00f, 0.08f, 0.18f, 0.30f, 0.42f, // rapid opening salvo
        0.55f, 0.65f, 0.75f,               // mid burst
        0.88f, 0.95f, 1.05f,               // climax
        1.20f, 1.45f, 1.70f,               // finale dying sparkle
        2.00f, 2.30f, 2.60f,               // EXTRA: more fireworks
        3.00f                              // EXTRA: final burst
    };
    static const int FW_WAVE_SITES[] = {
        2, 0, 3, 6, 1,
        4, 7, 5,
        8, 1, 0,
        9, 10, 3,
        2, 5, 8, // EXTRA launches
        1        // EXTRA launch
    };
    static int fwWaveDone[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static bool fwWaveReset = false;

    // Reset when newly activated
    if (fwTimer < dt * 2.0f)
    {
        for (int i = 0; i < 14; i++)
            fwWaveDone[i] = 0;
        fwWaveReset = true;
    }

    for (int w = 0; w < 14; w++)
    {
        if (!fwWaveDone[w] && fwTimer >= FW_WAVE_TIMES[w])
        {
            int siteIdx = FW_WAVE_SITES[w] % FW_SITE_COUNT;
            fwLaunchRocket(siteIdx);
            // Launch a second rocket from a nearby random site for density
            if (fwRand() > 0.40f)
                fwLaunchRocket((siteIdx + 2 + (int)(fwRand() * 3)) % FW_SITE_COUNT);
            fwWaveDone[w] = 1;
        }
    }

    // Stop fireworks after sound duration + a little tail
    // Stop fireworks after sound duration + a little tail
    if (fwTimer > FW_SOUND_DURATION + 1.5f)
    {
        // FORCE CLEANUP ALL PARTICLES AND ROCKETS
        for (int i = 0; i < FW_MAX_PARTICLES; i++)
        {
            fwParticles[i].active = false;
            fwParticles[i].life = 0.0f;
        }
        for (int i = 0; i < FW_MAX_ROCKETS; i++)
        {
            fwRockets[i].active = false;
            fwRockets[i].hasBurst = false;
        }

        fwActive = false;
        fwTimer = 0.0f;
        fwFlashAlpha = 0.0f;

        for (int w = 0; w < 14; w++)
            fwWaveDone[w] = 0;
    }

    float asp = 1400.0f / 800.0f;

    // Update rockets
    for (int i = 0; i < FW_MAX_ROCKETS; i++)
    {
        FWRocket &rk = fwRockets[i];
        if (!rk.active)
            continue;

        // Spawn trail smoke/sparks
        rk.trailTimer += dt;
        if (rk.trailTimer > 0.020f)
        {
            rk.trailTimer = 0.0f;
            // Bright spark trail
            FWParticle *tp = fwAllocParticle();
            tp->x = rk.x + fwRandSym() * 0.003f;
            tp->y = rk.y - 0.004f;
            tp->vx = fwRandSym() * 0.0015f;
            tp->vy = -0.003f - fwRand() * 0.003f;
            tp->ax = 0.0f;
            tp->ay = 0.00005f;
            tp->life = 0.35f;   // Balanced life for trails
            tp->decay = 0.045f; // Normal decay
            tp->size = 0.003f + fwRand() * 0.003f;
            tp->r = rk.r;
            tp->g = rk.g;
            tp->b = rk.b;
            tp->isTrail = true;
            tp->isSpark = false;
            tp->active = true;
            tp->strobePhase = 0;
            // Smoke puff
            fwSpawnTrailPuff(rk.x, rk.y, 120, 115, 110);
        }

        // Move rocket
        rk.x += rk.vx;
        rk.y += rk.vy;

        // Check if reached burst point
        float dy = rk.ty - rk.y;
        float dx = rk.tx - rk.x;
        float distSq = dx * dx + dy * dy;

        // Also check if it overshot (dot product sign flip)
        bool reached = (distSq < 0.0010f) ||
                       ((rk.vx * dx + rk.vy * dy) < 0.0f);

        if (reached && !rk.hasBurst)
        {
            rk.hasBurst = true;
            rk.active = false;
            fwBurst(rk.x, rk.y, rk.type,
                    rk.r, rk.g, rk.b,
                    rk.r2, rk.g2, rk.b2);
        }

        // Safety deactivate if rocket goes off screen
        if (rk.y > 1.05f || rk.x < -1.2f || rk.x > 1.2f)
        {
            rk.active = false;
            if (!rk.hasBurst)
            {
                rk.hasBurst = true;
                fwBurst(rk.x, fminf(rk.y, 0.98f),
                        rk.type,
                        rk.r, rk.g, rk.b,
                        rk.r2, rk.g2, rk.b2);
            }
        }
    }

    // Update particles
    // Update particles
    static int strobeCounter = 0;
    strobeCounter++;

    for (int i = 0; i < FW_MAX_PARTICLES; i++)
    {
        FWParticle &p = fwParticles[i];
        if (!p.active)
            continue;

        // Physics
        p.vx += p.ax;
        p.vy += p.ay;
        p.x += p.vx;
        p.y += p.vy;
        p.life -= p.decay;

        // Aggressive cleanup - remove particles with low life
        if (p.life <= 0.12f)
        {
            p.active = false;
            continue;
        }

        // Extra cleanup for trail particles (they have different alpha behavior)
        if (p.isTrail && p.life <= 0.10f)
        {
            p.active = false;
            continue;
        }

        // Secondary sparkle spawning
        if (p.isSpark && p.life < 0.35f && p.life > 0.30f)
        {
            p.isSpark = false;
            for (int s = 0; s < 5; s++)
            {
                FWParticle *sp = fwAllocParticle();
                sp->x = p.x;
                sp->y = p.y;
                sp->vx = fwRandSym() * 0.003f;
                sp->vy = fwRandSym() * 0.003f;
                sp->ax = 0.0f;
                sp->ay = -0.00015f;
                sp->life = 0.18f;
                sp->decay = 0.055f; // Fast decay
                sp->size = 0.0018f + fwRand() * 0.002f;
                sp->r = 255;
                sp->g = 230;
                sp->b = 100;
                sp->isTrail = false;
                sp->isSpark = false;
                sp->active = true;
                sp->strobePhase = 0;
            }
        }
    }
    // Stop fireworks after sound duration + a little tail
    if (fwTimer > FW_SOUND_DURATION + 1.5f)
    {
        fwForceCleanup(); // ← Add this line
        // Remove the individual cleanup lines below
    }
}
// Draw fireworks screen flash
static void drawFWFlash()
{
    if (fwFlashAlpha < 0.01f)
        return;
    glColor4f(1.0f, 1.0f, 1.0f, fwFlashAlpha * 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
    fwFlashAlpha *= 0.75f; // decay
    if (fwFlashAlpha < 0.01f)
        fwFlashAlpha = 0.0f;
}

// Draw fireworks
static void drawFireworks()
{
    // Always draw remaining particles even when !fwActive
    bool anyAlive = false;
    for (int i = 0; i < FW_MAX_PARTICLES; i++)
    {
        if (fwParticles[i].active)
        {
            anyAlive = true;
            break;
        }
    }
    bool anyRocket = false;
    for (int i = 0; i < FW_MAX_ROCKETS; i++)
    {
        if (fwRockets[i].active)
        {
            anyRocket = true;
            break;
        }
    }
    if (!fwActive && !anyAlive && !anyRocket)
        return;

    float asp = 1400.0f / 800.0f;
    static int strobeCounter = 0;
    strobeCounter++;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw particles
    for (int i = 0; i < FW_MAX_PARTICLES; i++)
    {
        FWParticle &p = fwParticles[i];
        if (!p.active)
            continue;

        float lifeN = fmaxf(0.0f, p.life); // 0..1 normalised

        if (p.isTrail)
        {
            // Smoke/trail puff — grey, large, very transparent
            unsigned char a = (unsigned char)(lifeN * lifeN * 80.0f);
            if (a < 3)
                continue;
            glColor4ub(p.r, p.g, p.b, a);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(p.x, p.y);
            int segs = 8;
            for (int s = 0; s <= segs; s++)
            {
                float ang = s * 2.0f * PI / segs;
                glVertex2f(p.x + cosf(ang) * p.size,
                           p.y + sinf(ang) * p.size * 0.6f);
            }
            glEnd();
            continue;
        }

        // Strobe — flicker effect
        bool strobeOn = true;
        if (p.strobePhase != 0)
        {
            strobeOn = ((strobeCounter + p.strobePhase) % 4) < 2;
        }
        if (!strobeOn)
            continue;

        // Alpha: bright for first 60% of life, then fades fast
        float alphaN;
        if (lifeN > 0.60f)
            alphaN = 1.0f;
        else
            alphaN = lifeN / 0.60f;
        alphaN = alphaN * alphaN; // quadratic fade

        unsigned char a = (unsigned char)(alphaN * 245.0f);
        if (a < 8)
            continue; // Low threshold - particles fade out naturally

        // Glow halo
        {
            unsigned char ga = (unsigned char)(alphaN * 55.0f);
            glColor4ub(p.r, p.g, p.b, ga);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(p.x, p.y);
            float haloSz = p.size * 5.0f;
            for (int s = 0; s <= 10; s++)
            {
                float ang = s * 2.0f * PI / 10.0f;
                glColor4ub(p.r, p.g, p.b, 0);
                glVertex2f(p.x + cosf(ang) * haloSz / asp,
                           p.y + sinf(ang) * haloSz);
            }
            glEnd();
        }

        // Core bright dot
        glColor4ub(p.r, p.g, p.b, a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(p.x, p.y);
        for (int s = 0; s <= 10; s++)
        {
            float ang = s * 2.0f * PI / 10.0f;
            unsigned char cr = (unsigned char)(fminf(255.0f, p.r * 1.3f));
            unsigned char cg = (unsigned char)(fminf(255.0f, p.g * 1.3f));
            unsigned char cb = (unsigned char)(fminf(255.0f, p.b * 1.3f));
            glColor4ub(cr, cg, cb, (unsigned char)(a * 0.5f));
            glVertex2f(p.x + cosf(ang) * p.size / asp,
                       p.y + sinf(ang) * p.size);
        }
        glEnd();

        // Short velocity trail
        {
            float trailLen = 2.8f;
            float tx = p.x - p.vx * trailLen;
            float ty = p.y - p.vy * trailLen;
            unsigned char ta = (unsigned char)(alphaN * 160.0f);
            glColor4ub(p.trailR, p.trailG, p.trailB, ta);
            glLineWidth(1.2f);
            glBegin(GL_LINES);
            glVertex2f(p.x, p.y);
            glColor4ub(p.trailR, p.trailG, p.trailB, 0);
            glVertex2f(tx, ty);
            glEnd();
        }
    }

    // Draw rockets (ascending phase)
    for (int i = 0; i < FW_MAX_ROCKETS; i++)
    {
        FWRocket &rk = fwRockets[i];
        if (!rk.active)
            continue;

        // Rocket body — bright dot
        glColor4ub(rk.r, rk.g, rk.b, 240);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(rk.x, rk.y);
        float rSz = 0.006f;
        for (int s = 0; s <= 12; s++)
        {
            float ang = s * 2.0f * PI / 12.0f;
            glColor4ub(255, 255, 220, 80);
            glVertex2f(rk.x + cosf(ang) * rSz / asp,
                       rk.y + sinf(ang) * rSz);
        }
        glEnd();

        // Rocket glow
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(rk.r, rk.g, rk.b, 70);
        glVertex2f(rk.x, rk.y);
        float gSz = 0.020f;
        for (int s = 0; s <= 12; s++)
        {
            float ang = s * 2.0f * PI / 12.0f;
            glColor4ub(rk.r, rk.g, rk.b, 0);
            glVertex2f(rk.x + cosf(ang) * gSz / asp,
                       rk.y + sinf(ang) * gSz);
        }
        glEnd();
    }

    glLineWidth(1.0f);
}

// ── Additional helpers for smooth day/night transition ──
static float smoothStep(float t) //[Prottoy]
{
    t = fmaxf(0.f, fminf(1.f, t));
    return t * t * t * (t * (t * 6.f - 15.f) + 10.f); // 5th-order (Ken Perlin) — much smoother
}

static float getDayBlend() //[Prottoy]
{
    if (!isDay)
        return 0.f;
    // Used sin arc but applied smoothstep on the normalized [0,1] value
    float raw = sinf(dayAngle);
    raw = fmaxf(0.f, raw);
    return smoothStep(raw);
}

static float getTwilightBlend()
{
    float a = isDay ? dayAngle : nightAngle;
    // Twilight = close to horizon (sin(a) near 0)
    float sinA = sinf(a);
    float nearHorizon = 1.f - fabsf(sinA);
    nearHorizon = fmaxf(0.f, nearHorizon);
    // Narrow the band so twilight is only ±15° from horizon
    float band = powf(nearHorizon, 3.5f);
    float strength = isDay ? 1.f : 0.55f;
    return smoothStep(band * strength); // depends on the near horizon when sun is in the top of the height the near horizon will  be 0
}

// Internal helpers
static float lerpF(float a, float b, float t) { return a + (b - a) * t; }
static float getDaylight() //[Prottoy]
{
    return getDayBlend();
}

static float getTwilight() //[Prottoy]
{
    return getTwilightBlend();
}
static void getSunPos(float &sx, float &sy) //[Prottoy]
{
    sx = -cosf(dayAngle) * 0.82f;
    sy = 0.04f + sinf(dayAngle) * 0.79f; // sun arch
}

static void getMoonPos(float &mx, float &my) //[Prottoy]
{
    mx = -cosf(nightAngle) * 0.75f;
    my = 0.04f + sinf(nightAngle) * 0.72f; // moon arch
}

struct Petal
{
    float x, y;
    float vx, vy;
    float angle;
    float angSpeed;
    float size;
    float swing;
    float swingSpeed;
    float swingAmp;
    bool active;
    int colorType; // 0 = pink sakura, 1 = white blossom
};

struct RestingPetal
{
    float x, y;
    float angle;
    float size;
    float alpha;
    int colorType; // 0 = pink, 1 = white
};

static Petal fallingPetals[MAX_FALLING];
static RestingPetal restingPetals[MAX_RESTING];
static int restingCount = 0;

// ─────────────────────────────────────────────────────────────────
//  Emad — moving cars on the road
//  Eight cars total: four in each lane. Each car only needs its
//  current X position and a colour index. Y, speed and direction
//  are decided from the lane it is in.
// ─────────────────────────────────────────────────────────────────
static float carX[8];   // current X position of each car
static int carColor[8]; // 0..5 colour code for each car
static int carLane[8];  // 0 = top (going right), 1 = bottom (going left)

// ─────────────────────────────────────────────────────────────────
//  Emad — moving pedestrians on the footpath
//  Six pedestrians total. Each one only needs current X, the row
//  it is walking on (top kerb or bottom kerb) and a colour code.
// ─────────────────────────────────────────────────────────────────
static float pedX[6];   // current X position of each pedestrian
static int pedRow[6];   // 0 = top kerb row, 1 = bottom kerb row
static int pedColor[6]; // 0..5 colour code for each pedestrian
static const float blossomX[] = {-0.38f, 0.34f, -0.85f, 0.41f, -0.56f, 0.16f, 0.84f, -0.31f, 0.23f, 0.95f};
static const int nBlossom = 10;

// Simple deterministic pseudo-random (no srand needed)
static int petalSeed = 42;
static float petalRand()
{
    petalSeed = petalSeed * 1664525 + 1013904223;
    return ((petalSeed & 0x7FFF) / (float)0x7FFF); // 0.0 – 1.0
}
#define RGB255(r, g, b) ((r) / 255.0f), ((g) / 255.0f), ((b) / 255.0f)
static void solidQ(float x1, float y1, float x2, float y2, unsigned char r, unsigned char g, unsigned char b)
{
    glColor3ub(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}
// Draw a semi‑transparent quad
static void alphaQ(float x1, float y1, float x2, float y2, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void diskFan(float cx, float cy, float rx, float ry, int seg, unsigned char r, unsigned char g, unsigned char b)
{
    glColor3ub(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++)
    {
        float a = i * 2.f * PI / seg;
        glVertex2f(cx + cosf(a) * rx, cy + sinf(a) * ry); // I can use ellipse shape with this function.
    }
    glEnd();
}

// Simple rectangle wrapper (used for roadside items)
static void rect(float x1, float y1, float x2, float y2, unsigned char r, unsigned char g, unsigned char b)
{
    solidQ(x1, y1, x2, y2, r, g, b);
}

// Filled circle (used for foliage and lamp glow)
static void fillCircle(float cx, float cy, float rx, float ry, unsigned char r, unsigned char g, unsigned char b)
{
    diskFan(cx, cy, rx, ry, 20, r, g, b);
}

// ================================================================
//  LEFT BACKGROUND BUILDINGS HELPERS
// ================================================================
static void drawRectBG(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void drawBuilding(float left, float bottom, float right, float top, int winCols, int winRows, int greyR, int greyG, int greyB)
{
    float bw = right - left, bh = top - bottom;
    glColor3f(greyR / 255.f, greyG / 255.f, greyB / 255.f);
    drawRectBG(left, bottom, right, top);
    glColor3f((greyR - 18) / 255.f, (greyG - 18) / 255.f, (greyB - 16) / 255.f);
    drawRectBG(left, top - 0.025f, right, top);
    float winW = (bw * 0.55f) / winCols, winH = (bh * 0.50f) / winRows;
    float xGap = (bw - winCols * winW) / (winCols + 1.f);
    float yGap = (bh - winRows * winH) / (winRows + 1.f);
    glColor3f(RGB255(80, 148, 210));
    for (int r = 0; r < winRows; r++)
        for (int c = 0; c < winCols; c++)
        {
            float wx = left + xGap + c * (winW + xGap);
            float wy = bottom + yGap + r * (winH + yGap);
            drawRectBG(wx, wy, wx + winW, wy + winH);
        }
}

// ================================================================
//  D-BUILDING SUB-FUNCTIONS
// ================================================================
static void drawLoungeTerrace(float vX, float vY, float vW, float vH, float hangH)
{
    solidQ(vX - 0.015f, vY, vX + vW + 0.015f, vY + vH * 0.26f, 60, 50, 35);               // ground layer ->brown
    solidQ(vX - 0.015f, vY - 0.005f, vX + vW + 0.015f, vY + vH * 0.30f, 48, 128, 38);     // middle layer
    solidQ(vX - 0.015f, vY + vH * 0.12f, vX + vW + 0.015f, vY + vH * 0.18f, 32, 108, 28); // Base layers have been created with this quads.
    int nP = 24;
    for (int i = 0; i < nP; i++)
    {
        float px = vX + (i + 0.5f) * vW / (float)nP;
        float ph = vH * (0.50f + 0.30f * ((i * 11 + 3) % 9) / 8.f);
        float pw = vW / (float)nP * 0.90f;
        glColor3ub(88, 64, 36);
        glBegin(GL_QUADS);
        glVertex2f(px + pw * 0.42f, vY + vH * 0.18f);
        glVertex2f(px + pw * 0.58f, vY + vH * 0.18f);
        glVertex2f(px + pw * 0.58f, vY + vH * 0.18f + ph * 0.30f); // pots for trees
        glVertex2f(px + pw * 0.42f, vY + vH * 0.18f + ph * 0.30f); // Detailing vertical quads have been created
        glEnd();
        int sh = i % 4;
        glColor3ub(32 + sh * 10, 112 + sh * 8, 26 + sh * 6);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(px + pw * 0.5f, vY + vH * 0.18f + ph);
        for (int k = 0; k <= 16; k++)
        {
            float a = k * 2.f * PI / 16.f;
            glVertex2f(px + pw * 0.5f + cosf(a) * pw * 0.58f,
                       vY + vH * 0.18f + ph * 0.55f + sinf(a) * ph * 0.44f); // Created the dome like trees.
        }
        glEnd();
        if (i % 3 == 0)
        {
            glColor3ub(68, 168, 48);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(px + pw * 0.5f - 0.003f, vY + vH * 0.18f + ph - 0.006f);
            for (int k = 0; k <= 10; k++)
            {
                float a = k * 2.f * PI / 10.f;
                glVertex2f(px + pw * 0.5f - 0.003f + cosf(a) * pw * 0.30f,
                           vY + vH * 0.18f + ph - 0.006f + sinf(a) * ph * 0.24f); // created flowers with small circles
            }
            glEnd();
        }
        if (i % 2 == 0)
        {
            glColor3ub(255, 140 + sh * 30, 180 - sh * 20);
            glPointSize(3.8f);
            glBegin(GL_POINTS);
            for (int fl = 0; fl < 3; fl++)
            {
                float fx = px + pw * 0.5f + (fl - 1) * pw * 0.15f;
                float fy = vY + vH * 0.18f + ph * 0.55f + (fl % 2 - 0.5f) * ph * 0.12f; // These flowers are here created by GL_Points
                glVertex2f(fx, fy);
            }
            glEnd();
        }
    }
    if (hangH > 0.f)
    {
        glColor3ub(38, 118, 30);
        glLineWidth(1.4f);
        for (int i = 0; i < 52; i++)
        {
            float vx2 = vX + (i + 0.35f) * vW / 52.f;
            float vl = hangH * (0.45f + 0.55f * ((i * 9 + 2) % 13) / 12.f); // 52 hangings vines were created, some with GL_Line stipes and some with the circles
            glBegin(GL_LINE_STRIP);
            for (int k = 0; k <= 8; k++)
            {
                float t = (float)k / 8.f;
                glVertex2f(vx2 - 0.005f * t, vY - vl * t);
            }
            glEnd();
            if (i % 3 == 0)
            {
                glColor3ub(44, 138, 35);
                glBegin(GL_TRIANGLE_FAN);
                glVertex2f(vx2 - 0.005f, vY - vl);
                for (int k = 0; k <= 8; k++)
                {
                    float a = k * 2.f * PI / 8.f;
                    glVertex2f(vx2 - 0.005f + cosf(a) * 0.0045f,
                               vY - vl + sinf(a) * 0.0060f);
                }
                glEnd();
                glColor3ub(38, 118, 30);
            }
        }
    }
}

static void drawAIUB(float sX, float sY, float sW, float sH) //[Prottoy]
{
    solidQ(sX, sY, sX + sW, sY + sH, 18, 28, 54);
    glColor3ub(68, 84, 130);
    glLineWidth(1.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(sX, sY);
    glVertex2f(sX + sW, sY);
    glVertex2f(sX + sW, sY + sH);
    glVertex2f(sX, sY + sH);
    glEnd();
    glColor3ub(238, 244, 255);
    glLineWidth(2.4f);
    float ch = sH * 0.68f, cw = sW * 0.118f, cs = sW * 0.045f;
    float ax = sX + sW * 0.072f, ty0 = sY + sH * 0.19f;
    // A
    glBegin(GL_LINES);
    glVertex2f(ax, ty0);
    glVertex2f(ax + cw * 0.5f, ty0 + ch);
    glVertex2f(ax + cw, ty0);
    glVertex2f(ax + cw * 0.5f, ty0 + ch);
    glVertex2f(ax + cw * 0.23f, ty0 + ch * 0.48f);
    glVertex2f(ax + cw * 0.77f, ty0 + ch * 0.48f);
    glEnd();
    // I
    ax += cw + cs;
    glBegin(GL_LINES);
    glVertex2f(ax + cw * 0.5f, ty0);
    glVertex2f(ax + cw * 0.5f, ty0 + ch);
    glVertex2f(ax, ty0 + ch);
    glVertex2f(ax + cw, ty0 + ch);
    glVertex2f(ax, ty0);
    glVertex2f(ax + cw, ty0);
    glEnd();
    // U
    ax += cw + cs;
    glBegin(GL_LINES);
    glVertex2f(ax, ty0 + ch);
    glVertex2f(ax, ty0 + ch * 0.28f);
    glVertex2f(ax + cw, ty0 + ch);
    glVertex2f(ax + cw, ty0 + ch * 0.28f);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int k = 0; k <= 14; k++)
    {
        float a = PI + k * PI / 14.f;
        glVertex2f(ax + cw * 0.5f + cosf(a) * cw * 0.5f, ty0 + ch * 0.28f + sinf(a) * ch * 0.28f);
    }
    glEnd();
    // B
    ax += cw + cs;
    glBegin(GL_LINES);
    glVertex2f(ax, ty0);
    glVertex2f(ax, ty0 + ch);
    glVertex2f(ax, ty0 + ch);
    glVertex2f(ax + cw * 0.56f, ty0 + ch);
    glVertex2f(ax, ty0 + ch * 0.50f);
    glVertex2f(ax + cw * 0.60f, ty0 + ch * 0.50f);
    glVertex2f(ax, ty0);
    glVertex2f(ax + cw * 0.62f, ty0);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int k = 0; k <= 12; k++)
    {
        float a = -PI * 0.5f + k * PI / 12.f;
        glVertex2f(ax + cw * 0.56f + cosf(a) * cw * 0.34f, ty0 + ch * 0.74f + sinf(a) * ch * 0.26f);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int k = 0; k <= 12; k++)
    {
        float a = -PI * 0.5f + k * PI / 12.f;
        glVertex2f(ax + cw * 0.58f + cosf(a) * cw * 0.36f, ty0 + ch * 0.24f + sinf(a) * ch * 0.26f);
    }
    glEnd();
}
static void drawRightAnnex(float mainRight, float by, float bh, float fH, int NF) //[Prottoy]
{
    float aX = mainRight + 0.008f, aW = 0.095f, aH = bh * 0.99f, aY = by, aTop = aY + aH; // all necessary variables are decleared
    glColor3ub(36, 48, 62);
    glBegin(GL_QUADS);
    glVertex2f(aX, aY);
    glVertex2f(aX + aW, aY);
    glVertex2f(aX + aW, aTop);
    glVertex2f(aX, aTop); // dark grey background created
    glEnd();
    glColor4ub(62, 90, 135, 55);
    glBegin(GL_QUADS);
    glVertex2f(aX, aY + aH * 0.40f);
    glVertex2f(aX + aW, aY + aH * 0.40f); // slight lihgter transparent blue applied
    glVertex2f(aX + aW, aTop);
    glVertex2f(aX, aTop);
    glEnd();
    glColor4ub(175, 205, 245, 22);
    glBegin(GL_QUADS);
    glVertex2f(aX + aW * 0.08f, aY + aH * 0.05f);
    glVertex2f(aX + aW * 0.30f, aY + aH * 0.05f); // another layer applied
    glVertex2f(aX + aW * 0.26f, aTop);
    glVertex2f(aX + aW * 0.08f, aTop);
    glEnd();
    glColor3ub(24, 32, 44);
    glLineWidth(0.80f);
    int aCols = 10;
    for (int c = 1; c < aCols; c++)
    {
        float mx = aX + c * aW / (float)aCols;
        glBegin(GL_LINES);
        glVertex2f(mx, aY);
        glVertex2f(mx, aTop);
        glEnd(); // vertical mullions created
    }
    int aNF = (int)(aH / fH);
    for (int f = 0; f <= aNF; f++)
    {
        float fy = aY + f * fH;
        if (fy > aTop)
            break;
        solidQ(aX, fy, aX + aW, fy + 0.009f, 52, 62, 75);
        alphaQ(aX, fy + 0.009f, aX + aW, fy + 0.012f, 200, 220, 255, 18); // numbers of floor selected
    }
    glColor4ub(20, 28, 38, 140);
    glLineWidth(0.55f);
    for (int p = 0; p < aCols; p++)
    {
        float px = aX + p * aW / (float)aCols;
        glBegin(GL_LINES);
        glVertex2f(px, aY);
        glVertex2f(px, aTop);
        glEnd();
    }
    solidQ(aX - 0.006f, aY, aX + 0.010f, aTop, 155, 157, 153);
    alphaQ(aX - 0.002f, aY, aX + 0.004f, aTop, 255, 255, 255, 20);
    solidQ(aX + aW - 0.005f, aY, aX + aW + 0.014f, aTop, 155, 157, 153);
    solidQ(aX - 0.004f, aTop, aX + aW + 0.010f, aTop + 0.018f, 155, 158, 154);
    solidQ(aX - 0.004f, aTop + 0.016f, aX + aW + 0.010f, aTop + 0.022f, 140, 143, 138); // Top roof elements
    solidQ(aX + aW * 0.10f, aTop + 0.022f, aX + aW * 0.35f, aTop + 0.042f, 88, 92, 98);
    solidQ(aX + aW * 0.45f, aTop + 0.022f, aX + aW * 0.75f, aTop + 0.038f, 76, 80, 86);
    solidQ(aX, aTop - fH * 0.85f, aX + aW, aTop - fH * 0.70f, 44, 50, 58);
    glColor3ub(32, 38, 46);
    glLineWidth(0.65f);
    for (int lv = 1; lv <= 10; lv++)
    {
        float ly = aTop - fH * 0.85f + lv * (fH * 0.15f) / 11.f;
        glBegin(GL_LINES);
        glVertex2f(aX, ly);
        glVertex2f(aX + aW, ly);
        glEnd();
    }
    glColor3ub(22, 30, 42);
    glLineWidth(1.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(aX, aY);
    glVertex2f(aX + aW, aY);
    glVertex2f(aX + aW, aTop);
    glVertex2f(aX, aTop);
    glEnd();
}

//  ANNEX-9 FOREGROUND BUILDING [Shajmin]
//
//  The whole building is first designed inside a local box from
//  (-0.85, -0.42) at the bottom-left corner to (+0.85, +0.80) at
//  the top-right corner — almost the full -1..+1 working area.
//  After everything is drawn at that big size, glScalef shrinks
//  it down and glTranslatef moves it into the correct spot in
//  the final scene.

//  a9Rect — draws a filled rectangle from (x1,y1) to (x2,y2)
static void a9Rect(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); // bottom-left corner
    glVertex2f(x2, y1); // bottom-right corner
    glVertex2f(x2, y2); // top-right corner
    glVertex2f(x1, y2); // top-left corner
    glEnd();
}

//  a9Window  — draws ONE single window (frame + glass)
//
//  Step 1: draws an outer grey rectangle = the window frame
//  Step 2: draws a slightly smaller blue rectangle inside
//          the frame = the glass.  "pad" is the thickness of
//          the frame on every side.
static void a9Window(float x, float y, float w, float h)
{
    const float pad = 0.014f; // frame thickness on each side

    // Step 1 — outer grey frame
    glColor3ub(105, 110, 122);
    a9Rect(x, y, x + w, y + h);

    // Step 2 — inner blue glass (smaller by 'pad' on every side)
    glColor3ub(88, 152, 218);
    a9Rect(x + pad, y + pad, x + w - pad, y + h - pad);
}

//  drawAnnex9  — main function that builds Annex-9
//
//  IMPORTANT: everything drawn between glPushMatrix() and
//  glPopMatrix() is affected by the translate + scale that
//  comes right after glPushMatrix().  So the coordinates we
//  write inside (like -0.85, +0.80) are the LOCAL coordinates
//  that get shrunk and shifted into world position by OpenGL.
//
//  glPushMatrix()  : save the current drawing state
//  glTranslatef()  : move the origin to where Annex-9 should sit
//  glScalef()      : shrink the local -1..+1 design to its real size
//  glPopMatrix()   : restore the saved state, so other drawings
//                    are not affected by the translate/scale
// ----------------------------------------------------------------
static void drawAnnex9()
{
    glPushMatrix();
    glTranslatef(-0.445f, -0.2085f, 0.0f); // move into final position
    glScalef(0.35235f, 0.36066f, 1.0f);    // shrink the local design

    // -------- Main facade (one big light-grey rectangle) --------
    glColor3ub(226, 229, 232);
    a9Rect(-0.85f, -0.42f, 0.85f, 0.80f);

    // -------- Horizontal floor-divider strips(white building L to R) --------
    // divY[] holds the Y-coordinate where each floor band starts.
    // Six bands evenly placed up the building.
    float divY[] = {-0.23f, -0.06f, 0.11f, 0.28f, 0.45f, 0.62f};
    glColor3ub(188, 192, 197);
    for (int i = 0; i < 6; i++)
    {
        // each band is a thin rectangle, height = 0.018
        a9Rect(-0.85f, divY[i], 0.85f, divY[i] + 0.018f);
    }

    //  Two vertical wing seams (separates the blue tower white building ) --------
    // Left seam between left wing and centre tower
    a9Rect(-0.584f, -0.42f, -0.566f, 0.80f);
    // Right seam between centre tower and right wing
    a9Rect(0.566f, -0.42f, 0.584f, 0.80f);

    //  Roof caps over the two side wings
    glColor3ub(148, 152, 160);
    a9Rect(-0.74f, 0.80f, -0.41f, 0.90f); // left roof cap(boottom L corner and top right corner)
    a9Rect(0.41f, 0.80f, 0.74f, 0.90f);   // right roof cap

    // Central blue glass tower
    glColor3ub(26, 48, 106);
    a9Rect(-0.30f, -0.42f, 0.30f, 0.80f); // dark blue base of tower

    // Lighter blue stripe on the left edge (looks like a side panel)
    glColor3ub(45, 76, 138);
    a9Rect(-0.28f, -0.42f, -0.22f, 0.80f);

    // horizontal line inside blue tower(re-uses the same divY[] heights)
    glColor3ub(34, 62, 120);
    for (int i = 0; i < 6; i++)
    {
        a9Rect(-0.29f, divY[i], 0.29f, divY[i] + 0.012f);
    }

    // Thin vertical line down the centre of the blue tower
    a9Rect(-0.009f, -0.42f, 0.009f, 0.80f);

    // -------- Main entrance at the bottom of the tower --------
    glColor3ub(150, 155, 165); // outer grey door frame
    a9Rect(-0.12f, -0.42f, 0.12f, -0.22f);
    glColor3ub(18, 35, 80); // dark blue door glass
    a9Rect(-0.10f, -0.42f, 0.10f, -0.24f);

    // -------- Window grid on the two side wings --------
    // We draw 5 rows × 2 columns of windows on EACH wing (left + right).
    //   winW, winH : size of a single window
    //   lcx[]      : X-coordinates of the 2 columns on the LEFT wing
    //   rcx[]      : X-coordinates of the 2 columns on the RIGHT wing
    //   rowY[]     : Y-coordinates of the 5 rows (same for both wings)
    const float winW = 0.18f;
    const float winH = 0.12f;
    const float lcx[] = {-0.79f, -0.54f}; // left Wbuilding (left wing er L col window & R col window)
    const float rcx[] = {0.36f, 0.61f};   // right  Wbuilding (right wing er L col window & R col window)
    const float rowY[] = {-0.32f, -0.09f, 0.13f, 0.35f, 0.57f};

    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 2; col++)
        {
            // window on the LEFT wing at column 'col', row 'row'
            a9Window(lcx[col], rowY[row], winW, winH);
            // matching window on the RIGHT wing at the same row
            a9Window(rcx[col], rowY[row], winW, winH);
        }
    }

    // -------- Outline around the whole facade for a clean edge --------
    glColor3ub(160, 163, 168);
    glLineWidth(1.2f);
    glBegin(GL_LINE_LOOP); // edge beside whole building
    glVertex2f(-0.85f, -0.42f);
    glVertex2f(0.85f, -0.42f);
    glVertex2f(0.85f, 0.80f);
    glVertex2f(-0.85f, 0.80f);
    glEnd();

    glPopMatrix(); // restore the matrix so other drawings are normal
}

//  PARKED CARS IN THE LOT                                 [Shajmin]
//
//  Each parked car is first designed inside a local box from
//  (-1, -1) to (+1, +1) by drawCarLocal(), the same idea Annex-9
//  uses.  Then for every car we want, we call drawParkedCar()
//  with a centre point and a size, and that function does the
//  push / translate / scale / draw / pop for us.  This way every
//  car is drawn the same way and we just place them by giving
//  centre coordinates.

//  drawCarLocal  — draws ONE car inside the local -1..+1 box.
//  Colour is passed in so the same code can draw any car colour.
//  The car faces "down" the page (windscreen at the bottom of
//  the local box) so the bay-row at the top will look like a car
//  parked nose-out.
static void drawCarLocal(unsigned char r, unsigned char g, unsigned char b)
{
    // Body — big coloured rectangle filling most of the local box
    glColor3ub(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(-0.85f, -0.80f);
    glVertex2f(0.85f, -0.80f);
    glVertex2f(0.85f, 0.80f);
    glVertex2f(-0.85f, 0.80f);
    glEnd();

    // Roof — a slightly smaller darker rectangle on top of the body
    glColor3ub(40, 40, 45);
    glBegin(GL_QUADS);
    glVertex2f(-0.55f, -0.45f);
    glVertex2f(0.55f, -0.45f);
    glVertex2f(0.55f, 0.40f);
    glVertex2f(-0.55f, 0.40f);
    glEnd();

    // Windscreen — blue strip at the bottom edge of the roof (front)
    glColor3ub(110, 170, 220);
    glBegin(GL_QUADS);
    glVertex2f(-0.50f, -0.45f);
    glVertex2f(0.50f, -0.45f);
    glVertex2f(0.50f, -0.10f);
    glVertex2f(-0.50f, -0.10f);
    glEnd();

    // Four wheels — small dark circles, one near each corner
    glColor3ub(20, 20, 22);
    diskFan(-0.65f, -0.55f, 0.18f, 0.10f, 10, 20, 20, 22);
    diskFan(0.65f, -0.55f, 0.18f, 0.10f, 10, 20, 20, 22);
    diskFan(-0.65f, 0.55f, 0.18f, 0.10f, 10, 20, 20, 22);
    diskFan(0.65f, 0.55f, 0.18f, 0.10f, 10, 20, 20, 22);
}

// ----------------------------------------------------------------
//  drawParkedCar  — places ONE car at (cx, cy) in the world with
//  the requested size, by scaling and translating drawCarLocal().
//  The exact same push / translate / scale / pop pattern that
//  Annex-9 uses.
// ----------------------------------------------------------------
static void drawParkedCar(float cx, float cy, float halfW, float halfH,
                          unsigned char r, unsigned char g, unsigned char b)
{
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);   // move the local origin to (cx, cy)
    glScalef(halfW, halfH, 1.0f); // shrink local -1..+1 down to this size
    drawCarLocal(r, g, b);        // now draw the car at local size
    glPopMatrix();                // restore matrix for the next thing
}

// ----------------------------------------------------------------
//  drawParkedCars  — calls drawParkedCar() once for every car
//  we want in the lot.  No loops, no arrays, no palettes:
//  every car is one explicit line of code, easy to read and
//  easy to change a single colour or position.
//
//  Layout reminder (matches drawParkingLot()):
//    Top row Y centre ≈ -0.430
//    Bot row Y centre ≈ -0.652
//    Bays span X from about -0.85 up to -0.16
//    Each car half-width ≈ 0.027, half-height ≈ 0.040
// ----------------------------------------------------------------
static void drawParkedCars()
{
    const float topY = -0.430f;
    const float botY = -0.652f;
    const float w = 0.027f; // car half-width
    const float h = 0.040f; // car half-height

    // ---- Top row of parked cars (left → right) ----
    drawParkedCar(-0.86f, topY, w, h, 200, 40, 40);   // red
    drawParkedCar(-0.79f, topY, w, h, 40, 90, 200);   // blue
    drawParkedCar(-0.725f, topY, w, h, 220, 180, 40); // yellow
    drawParkedCar(-0.64f, topY, w, h, 60, 160, 60);   // green
    drawParkedCar(-0.57f, topY, w, h, 180, 180, 185); // silver
    drawParkedCar(-0.495f, topY, w, h, 200, 90, 30);  // orange
    drawParkedCar(-0.43f, topY, w, h, 90, 50, 170);   // purple
    drawParkedCar(-0.35f, topY, w, h, 40, 40, 42);    // black
    drawParkedCar(-0.275f, topY, w, h, 200, 40, 40);  // red
    // (rightmost two top-row spaces left empty — no cars there)

    // ---- Bottom row of parked cars (left → right) ----
    drawParkedCar(-0.86f, botY, w, h, 40, 40, 42);    // black
    drawParkedCar(-0.79f, botY, w, h, 60, 160, 60);   // green
    drawParkedCar(-0.725f, botY, w, h, 220, 180, 40); // yellow
    drawParkedCar(-0.64f, botY, w, h, 200, 40, 40);   // red
    drawParkedCar(-0.57f, botY, w, h, 180, 180, 185); // silver
    drawParkedCar(-0.495f, botY, w, h, 40, 90, 200);  // blue
    drawParkedCar(-0.43f, botY, w, h, 200, 90, 30);   // orange
    drawParkedCar(-0.35f, botY, w, h, 90, 50, 170);   // purple
    drawParkedCar(-0.275f, botY, w, h, 40, 40, 42);   // black
    // (rightmost two bottom-row spaces left empty — no cars there)
}

//  PARKING LOT BOOTH                                      [Shajmin]
//
//  The security booth that sits at the entrance of the parking
//  lot.  Same local-design pattern as Annex-9: drawn inside the
//  parking lot's local -1..+1 box (so it lines up with the lot),
//  then scaled and translated into place.
static void drawParkingExitBuilding()
{
    glPushMatrix();
    glTranslatef(-0.55f, -0.5405f, 0.0f); // move local origin to lot centre
    glScalef(0.45f, 0.1675f, 1.0f);

    // Booth body — light grey block on the left side of the lot
    glColor3ub(190, 195, 200);
    glBegin(GL_QUADS);
    glVertex2f(-0.99f, -0.10f);
    glVertex2f(-0.83f, -0.10f);
    glVertex2f(-0.83f, 0.55f);
    glVertex2f(-0.99f, 0.55f);
    glEnd();
    // Booth roof
    glColor3ub(70, 75, 85);
    glBegin(GL_QUADS);
    glVertex2f(-1.00f, 0.55f);
    glVertex2f(-0.82f, 0.55f);
    glVertex2f(-0.82f, 0.65f);
    glVertex2f(-1.00f, 0.65f);
    glEnd();
    // Booth window — one single blue rectangle (no grid)
    glColor3ub(90, 150, 210);
    glBegin(GL_QUADS);
    glVertex2f(-0.96f, 0.18f);
    glVertex2f(-0.86f, 0.18f);
    glVertex2f(-0.86f, 0.45f);
    glVertex2f(-0.96f, 0.45f);
    glEnd();

    glPopMatrix();
}

//  PARKING LOT ENTRY ARM (boom barrier)                  [Shajmin]
//
//  A single red quad that pivots open/closed at the entrance of
//  the parking lot.
//  parkingArmAngle =  0° → closed (horizontal)
//  parkingArmAngle ≈ 85° → open (nearly vertical)
//
//  The 'u' (open) and 'i' (close) keys set parkingArmTarget;
static void drawParkingArm() // [Shajmin]
{
    // Pivot point sits just to the right of the security booth.
    const float pivotX = -0.905f;
    const float pivotY = -0.510f;
    const float armLen = 0.105f;
    const float armHalfT = 0.008f;

    // Move local origin to the pivot, rotate by parkingArmAngle,
    // then draw the arm extending to the right from the origin.
    glPushMatrix();
    glTranslatef(pivotX, pivotY, 0.0f);
    glRotatef(parkingArmAngle, 0.0f, 0.0f, 1.0f); // [Shajmin]

    glColor3ub(220, 50, 50); // red
    glBegin(GL_QUADS);
    glVertex2f(0.0f, -armHalfT);
    glVertex2f(armLen, -armHalfT);
    glVertex2f(armLen, armHalfT);
    glVertex2f(0.0f, armHalfT);
    glEnd();

    glPopMatrix();
}

// ================================================================
//  PARKING LOT                                              [Emad]
//
//  Same idea as Annex-9: design the lot inside a local box from
//  (-1, -1) at the bottom-left corner to (+1, +1) at the top-right
//  corner, then glScalef + glTranslatef move it into place.
//
//  Local layout:
//    Gate area  : X  -1.00  →  -0.78   (at the LEFT end)
//    Parking    : X  -0.78  →  +1.00
//    Bottom bays: Y  -1.00  →  -0.32
//    Drive lane : Y  -0.32  →  +0.32
//    Top    bays: Y  +0.32  →  +1.00
//
//  The world placement maps this local box onto:
//    World X  -1.00 .. -0.10
//    World Y  -0.708 .. -0.373
// ================================================================
static void drawParkingLot()
{
    glPushMatrix();
    glTranslatef(-0.55f, -0.5405f, 0.0f); // move local origin to lot centre
    glScalef(0.45f, 0.1675f, 1.0f);       // shrink local -1..+1 to lot size

    // -------- ASPHALT BASE (the whole lot floor) --------
    glColor3ub(62, 64, 68);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // -------- LIGHTER STRIP DOWN THE DRIVE LANE --------
    glColor3ub(74, 76, 80);
    glBegin(GL_QUADS);
    glVertex2f(-0.78f, -0.32f);
    glVertex2f(1.0f, -0.32f);
    glVertex2f(1.0f, 0.32f);
    glVertex2f(-0.78f, 0.32f);
    glEnd();

    // -------- WHITE BAY DIVIDER LINES (top row) --------
    // 12 lines = 11 parking bays in the top row.
    glColor3ub(230, 232, 235);
    glLineWidth(1.2f);
    for (int i = 0; i <= 11; i++)
    {
        float bx = -0.78f + i * (1.78f / 11.0f);
        glBegin(GL_LINES);
        glVertex2f(bx, 0.34f);
        glVertex2f(bx, 0.95f);
        glEnd();
    }

    // -------- WHITE BAY DIVIDER LINES (bottom row) --------
    for (int i = 0; i <= 11; i++)
    {
        float bx = -0.78f + i * (1.78f / 11.0f);
        glBegin(GL_LINES);
        glVertex2f(bx, -0.95f);
        glVertex2f(bx, -0.34f);
        glEnd();
    }

    // -------- YELLOW DASHED CENTRE LINE in the drive lane --------
    glColor3ub(255, 220, 60);
    glLineWidth(1.4f);
    for (int i = 0; i < 9; i++)
    {
        float dx = -0.70f + i * 0.20f;
        glBegin(GL_LINES);
        glVertex2f(dx, 0.0f);
        glVertex2f(dx + 0.12f, 0.0f);
        glEnd();
    }

    // -------- OUTER LOT BORDER --------
    glColor3ub(110, 112, 118);
    glLineWidth(1.6f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glPopMatrix();
}

// ================================================================
//  C-BUILDING — SPHERICAL GLOBE (6 floors + metallic top)
// ================================================================
static void sphereBandFill(float cx, float cy, float srx, float sry,
                           float yBot, float yTop,
                           unsigned char r, unsigned char g, unsigned char b, unsigned char a,
                           int nPts = 120)
{
    float yB = fmaxf(yBot, cy - sry);
    float yT = fminf(yTop, cy + sry);
    if (yT <= yB)
        return;
    glColor4ub(r, g, b, a);
    glBegin(GL_POLYGON);
    for (int p = 0; p <= nPts; p++)
    {
        float t = (float)p / nPts;
        float yy = yB;
        float ratio = (yy - cy) / sry;
        float hw = srx * sqrtf(fmaxf(0.f, 1.f - ratio * ratio));
        glVertex2f(cx - hw + t * 2.f * hw, yy);
    }
    for (int p = nPts; p >= 0; p--)
    {
        float t = (float)p / nPts;
        float yy = yT;
        float ratio = (yy - cy) / sry;
        float hw = srx * sqrtf(fmaxf(0.f, 1.f - ratio * ratio));
        glVertex2f(cx - hw + t * 2.f * hw, yy);
    }
    glEnd();
}

static void sphereBandLine(float cx, float cy, float srx, float sry,
                           float yy, unsigned char r, unsigned char g, unsigned char b,
                           float lw = 1.5f, int nPts = 120)
{
    float ratio = (yy - cy) / sry;
    if (fabsf(ratio) >= 1.f)
        return;
    float hw = srx * sqrtf(fmaxf(0.f, 1.f - ratio * ratio));
    glColor3ub(r, g, b);
    glLineWidth(lw);
    glBegin(GL_LINE_STRIP);
    for (int p = 0; p <= nPts; p++)
    {
        float t = (float)p / nPts;
        glVertex2f(cx - hw + t * 2.f * hw, yy);
    }
    glEnd();
}

static void sphereVertLine(float cx, float cy, float srx, float sry,
                           float xPos, float yBot, float yTop,
                           unsigned char r, unsigned char g, unsigned char b, unsigned char a,
                           float lw = 0.55f)
{
    float ratio = (xPos - cx) / srx;
    if (fabsf(ratio) >= 0.98f)
        return;
    float sinA = sqrtf(fmaxf(0.f, 1.f - ratio * ratio));
    float ytop = cy + sry * sinA;
    float ybot = cy - sry * sinA;
    float drawBot = fmaxf(yBot, ybot + 0.003f);
    float drawTop = fminf(yTop, ytop - 0.003f);
    if (drawTop <= drawBot)
        return;
    glColor4ub(r, g, b, a);
    glLineWidth(lw);
    glBegin(GL_LINES);
    glVertex2f(xPos, drawBot);
    glVertex2f(xPos, drawTop);
    glEnd();
}

static void drawCBuilding()
{
    const float cx = -0.715f, podBot = -0.360f;
    const float podH = 0.168f, podRX = 0.132f, podTop = podBot + podH;
    const float srx = 0.190f, sry = 0.250f;
    const float cy = podTop + sry * 0.78f;
    const int SEG = 150;

    // ----- PODIUM -----
    glColor4ub(10, 12, 18, 70);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx + 0.012f, podBot - 0.008f);
    for (int i = 0; i <= 40; i++)
    {
        float a = i * PI / 40.f;
        glVertex2f(cx + cosf(a + PI) * (podRX + 0.022f), podBot - 0.010f + sinf(a + PI) * 0.014f);
    }
    glEnd();

    float pw_bot = podRX, pw_top = podRX;
    glColor3ub(195, 192, 183);
    glBegin(GL_POLYGON);
    glVertex2f(cx - pw_bot, podBot);
    glVertex2f(cx + pw_bot, podBot);
    glVertex2f(cx + pw_top, podTop);
    glVertex2f(cx - pw_top, podTop);
    glEnd();
    glColor4ub(0, 0, 0, 38);
    glBegin(GL_POLYGON);
    glVertex2f(cx - pw_bot, podBot);
    glVertex2f(cx - pw_bot + 0.032f, podBot);
    glVertex2f(cx - pw_top + 0.028f, podTop);
    glVertex2f(cx - pw_top, podTop);
    glEnd();
    glColor4ub(255, 255, 255, 12);
    glBegin(GL_POLYGON);
    glVertex2f(cx + pw_bot - 0.028f, podBot);
    glVertex2f(cx + pw_bot, podBot);
    glVertex2f(cx + pw_top, podTop);
    glVertex2f(cx + pw_top - 0.024f, podTop);
    glEnd();

    float pg1Y = podBot + podH * 0.65f, pg1H = podH * 0.24f;
    solidQ(cx - pw_top, pg1Y, cx + pw_top, pg1Y + pg1H, 22, 32, 45);
    alphaQ(cx - pw_top, pg1Y, cx - pw_top * 0.30f, pg1Y + pg1H, 120, 165, 220, 25);
    glColor4ub(15, 22, 35, 180);
    glLineWidth(0.7f);
    for (int w = 1; w <= 11; w++)
    {
        float wx = cx - pw_top + w * 2.f * pw_top / 12.f;
        glBegin(GL_LINES);
        glVertex2f(wx, pg1Y + 0.004f);
        glVertex2f(wx, pg1Y + pg1H - 0.004f);
        glEnd();
    }
    glBegin(GL_LINES);
    glVertex2f(cx - pw_top, pg1Y + pg1H * 0.5f);
    glVertex2f(cx + pw_top, pg1Y + pg1H * 0.5f);
    glEnd();

    float pg2Y = podBot + podH * 0.14f, pg2H = podH * 0.20f;
    solidQ(cx - pw_bot, pg2Y, cx + pw_bot, pg2Y + pg2H, 22, 32, 45);
    alphaQ(cx - pw_bot, pg2Y, cx - pw_bot * 0.35f, pg2Y + pg2H, 120, 165, 220, 20);
    glColor4ub(15, 22, 35, 180);
    glLineWidth(0.7f);
    for (int w = 1; w <= 9; w++)
    {
        float wx = cx - pw_bot + w * 2.f * pw_bot / 10.f;
        glBegin(GL_LINES);
        glVertex2f(wx, pg2Y + 0.004f);
        glVertex2f(wx, pg2Y + pg2H - 0.004f);
        glEnd();
    }

    glColor3ub(148, 143, 132);
    glLineWidth(1.0f);
    float podDivs[] = {podBot + podH * 0.14f, podBot + podH * 0.34f, podBot + podH * 0.65f, podBot + podH * 0.89f};
    for (int i = 0; i < 4; i++)
    {
        glBegin(GL_LINES);
        glVertex2f(cx - pw_top, podDivs[i]);
        glVertex2f(cx + pw_top, podDivs[i]);
        glEnd();
    }

    glColor4ub(138, 133, 122, 120);
    glLineWidth(0.55f);
    for (int v = 1; v <= 15; v++)
    {
        float vx = cx - pw_top + v * 2.f * pw_top / 16.f;
        glBegin(GL_LINES);
        glVertex2f(vx, pg1Y + pg1H + 0.002f);
        glVertex2f(vx, podTop - 0.002f);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(vx, pg2Y + pg2H + 0.002f);
        glVertex2f(vx, pg1Y - 0.002f);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(vx, podBot + 0.002f);
        glVertex2f(vx, pg2Y - 0.002f);
        glEnd();
    }

    float colFracs[] = {-0.78f, -0.46f, -0.15f, 0.15f, 0.46f, 0.78f};
    float colW = 0.013f, colH = podH * 0.58f;
    for (int c = 0; c < 6; c++)
    {
        float colX = cx + colFracs[c] * pw_top;
        if (colX < cx - pw_top + colW || colX > cx + pw_top - colW)
            continue;
        solidQ(colX - colW * 0.5f, podBot, colX + colW * 0.5f, podBot + colH, 196, 42, 38);
        alphaQ(colX - colW * 0.5f, podBot, colX - colW * 0.5f + 0.003f, podBot + colH, 255, 200, 180, 35);
        solidQ(colX - colW * 0.65f, podBot + colH - 0.008f, colX + colW * 0.65f, podBot + colH + 0.004f, 160, 36, 32);
    }

    solidQ(cx - pw_top - 0.009f, podTop, cx + pw_top + 0.009f, podTop + 0.014f, 172, 168, 160);
    alphaQ(cx - pw_top - 0.009f, podTop, cx + pw_top + 0.009f, podTop + 0.006f, 255, 255, 255, 22);
    glColor3ub(140, 136, 128);
    glLineWidth(1.2f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - pw_bot, podBot);
    glVertex2f(cx + pw_bot, podBot);
    glVertex2f(cx + pw_top, podTop);
    glVertex2f(cx - pw_top, podTop);
    glEnd();

    // ----- SPHERE (13 bands, top larger) -----
    float fY[14];
    float totalHeight = 2.0f * sry;
    float topBandHeight = 0.28f * sry;
    float lowerHeight = totalHeight - topBandHeight;
    fY[0] = cy - sry;
    for (int i = 1; i <= 12; i++)
        fY[i] = fY[0] + (i / 12.0f) * lowerHeight;
    fY[13] = cy + sry;

    const unsigned char mR = 212, mG = 208, mB = 198, gR = 28, gG = 38, gB = 52;

    glColor4ub(15, 18, 25, 60);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx + 0.014f, cy - 0.012f);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glVertex2f(cx + 0.014f + cosf(a) * (srx + 0.014f), cy - 0.012f + sinf(a) * (sry + 0.014f));
    }
    glEnd();

    glColor3ub(mR, mG, mB);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glVertex2f(cx + cosf(a) * srx, cy + sinf(a) * sry);
    }
    glEnd();

    glColor4ub(255, 252, 242, 68);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx + srx * 0.18f, cy + sry * 0.32f);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glVertex2f(cx + srx * 0.18f + cosf(a) * srx * 0.55f, cy + sry * 0.32f + sinf(a) * sry * 0.50f);
    }
    glEnd();

    glColor4ub(22, 24, 30, 50);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx - srx * 0.24f, cy - sry * 0.28f);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glVertex2f(cx - srx * 0.24f + cosf(a) * srx * 0.68f, cy - sry * 0.28f + sinf(a) * sry * 0.62f);
    }
    glEnd();

    for (int band = 1; band <= 13; band++)
    {
        if (band % 2 == 0)
        {
            unsigned char r = gR, g = gG, b = gB, a = 230;
            if (band % 4 == 0)
            {
                r = 32;
                g = 44;
                b = 64;
            }
            sphereBandFill(cx, cy, srx, sry, fY[band - 1], fY[band], r, g, b, a);
        }
    }

    for (int band = 1; band <= 13; band++)
    {
        if (band % 2 == 0)
        {
            float yB = fY[band - 1], yT = fY[band];
            float gY1 = yB + (yT - yB) * 0.05f, gY2 = yB + (yT - yB) * 0.48f;
            float r1 = (gY1 - cy) / sry, r2 = (gY2 - cy) / sry;
            float hw1 = srx * sqrtf(fmaxf(0.f, 1.f - r1 * r1));
            float hw2 = srx * sqrtf(fmaxf(0.f, 1.f - r2 * r2));
            glColor4ub(85, 130, 185, 35);
            glBegin(GL_POLYGON);
            glVertex2f(cx - hw1 * 0.90f, gY1);
            glVertex2f(cx - hw1 * 0.18f, gY1);
            glVertex2f(cx - hw2 * 0.16f, gY2);
            glVertex2f(cx - hw2 * 0.88f, gY2);
            glEnd();
            glColor4ub(200, 225, 255, 18);
            glBegin(GL_POLYGON);
            glVertex2f(cx + hw1 * 0.22f, gY1);
            glVertex2f(cx + hw1 * 0.65f, gY1);
            glVertex2f(cx + hw2 * 0.60f, gY2);
            glVertex2f(cx + hw2 * 0.18f, gY2);
            glEnd();
        }
    }

    int nLatLines = 26;
    glColor4ub(130, 124, 112, 145);
    glLineWidth(0.58f);
    for (int lat = 1; lat < nLatLines; lat++)
    {
        float yy = cy - sry + lat * 2.f * sry / nLatLines;
        sphereBandLine(cx, cy, srx, sry, yy, 125, 118, 106, 0.58f, 100);
    }

    for (int b = 0; b <= 13; b++)
    {
        float lw = (b == 0 || b == 13) ? 1.0f : 2.2f;
        unsigned char gr = (b >= 1 && b <= 12) ? 55 : 105;
        sphereBandLine(cx, cy, srx, sry, fY[b], gr, gr - 8, gr - 18, lw, 110);
    }

    int nLong = 22;
    for (int v = 0; v < nLong; v++)
    {
        float xPos = cx - srx + (v + 0.5f) * 2.f * srx / nLong;
        float ratio = (xPos - cx) / srx;
        if (fabsf(ratio) > 0.97f)
            continue;
        for (int band = 1; band <= 13; band++)
        {
            if (band % 2 == 0)
                sphereVertLine(cx, cy, srx, sry, xPos, fY[band - 1], fY[band], 30, 42, 60, 165, 0.55f);
            else
                sphereVertLine(cx, cy, srx, sry, xPos, fY[band - 1], fY[band], 125, 118, 106, 140, 0.58f);
        }
    }

    for (int band = 1; band <= 13; band++)
    {
        if (band % 2 == 0)
        {
            float yB = fY[band - 1], yT = fY[band];
            float midY = yB + (yT - yB) * 0.52f;
            float ratioM = (midY - cy) / sry;
            if (fabsf(ratioM) < 0.98f)
            {
                float hwM = srx * sqrtf(fmaxf(0.f, 1.f - ratioM * ratioM));
                glColor4ub(18, 28, 42, 200);
                glLineWidth(1.2f);
                glBegin(GL_LINE_STRIP);
                for (int p = 0; p <= 80; p++)
                {
                    float t = (float)p / 80;
                    glVertex2f(cx - hwM + t * 2.f * hwM, midY);
                }
                glEnd();
            }
            float intY = yB + (yT - yB) * 0.60f;
            float ratioI = (intY - cy) / sry;
            if (fabsf(ratioI) < 0.98f)
            {
                float hwI = srx * sqrtf(fmaxf(0.f, 1.f - ratioI * ratioI));
                glColor4ub(200, 210, 220, 14);
                glBegin(GL_POLYGON);
                float rB = (yB + (yT - yB) * 0.55f - cy) / sry;
                float hwB2 = srx * sqrtf(fmaxf(0.f, 1.f - rB * rB));
                for (int p = 0; p <= 60; p++)
                {
                    float t = (float)p / 60;
                    glVertex2f(cx - hwB2 + t * 2.f * hwB2, yB + (yT - yB) * 0.55f);
                }
                float rT = (yT - (yT - yB) * 0.08f - cy) / sry;
                float hwT2 = srx * sqrtf(fmaxf(0.f, 1.f - rT * rT));
                for (int p = 60; p >= 0; p--)
                {
                    float t = (float)p / 60;
                    glVertex2f(cx - hwT2 + t * 2.f * hwT2, yT - (yT - yB) * 0.08f);
                }
                glEnd();
            }
        }
    }

    glColor3ub(118, 112, 100);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glVertex2f(cx + cosf(a) * srx, cy + sinf(a) * sry);
    }
    glEnd();

    // ----- TREES AROUND PODIUM -----
    struct Tree
    {
        float ox;
        float h;
        int shade;
        float aspect;
    };
    Tree trees[] = {
        {-0.195f, 0.078f, 2, 1.1f},
        {-0.148f, 0.095f, 0, 0.9f},
        {-0.108f, 0.088f, 1, 1.2f},
        {-0.072f, 0.082f, 2, 1.0f},
        {-0.038f, 0.092f, 0, 1.1f},
        {0.005f, 0.086f, 1, 0.95f},
        {0.052f, 0.080f, 2, 1.05f},
        {0.095f, 0.074f, 0, 1.15f},
        {0.138f, 0.070f, 1, 1.0f},
        {0.175f, 0.065f, 2, 0.90f},
        {-0.168f, 0.065f, 1, 0.8f},
        {0.118f, 0.060f, 0, 0.85f},
    };
    int nT = 12;
    for (int t = 0; t < nT; t++)
    {
        float tx = cx + trees[t].ox, tH = trees[t].h, asp = trees[t].aspect;
        int sh = trees[t].shade;
        glColor3ub(72 + sh * 8, 52 + sh * 4, 24 + sh * 2);
        glBegin(GL_QUADS);
        glVertex2f(tx - 0.005f, podBot - tH * 0.50f);
        glVertex2f(tx + 0.005f, podBot - tH * 0.50f);
        glVertex2f(tx + 0.004f, podBot);
        glVertex2f(tx - 0.004f, podBot);
        glEnd();
        float fR = 28 + sh * 12, fG = 92 + sh * 18, fB = 24 + sh * 8;
        glColor3ub((unsigned char)fR, (unsigned char)fG, (unsigned char)fB);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(tx, podBot - tH * 0.50f + tH);
        for (int k = 0; k <= 18; k++)
        {
            float a = k * 2.f * PI / 18.f;
            glVertex2f(tx + cosf(a) * tH * 0.54f * asp,
                       podBot - tH * 0.50f + tH * 0.46f + sinf(a) * tH * 0.48f);
        }
        glEnd();
        glColor3ub((unsigned char)(fR + 14), (unsigned char)(fG + 20), (unsigned char)(fB + 8));
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(tx + tH * 0.05f, podBot - tH * 0.50f + tH * 1.05f);
        for (int k = 0; k <= 14; k++)
        {
            float a = k * 2.f * PI / 14.f;
            glVertex2f(tx + tH * 0.05f + cosf(a) * tH * 0.34f * asp,
                       podBot - tH * 0.50f + tH * 0.80f + sinf(a) * tH * 0.30f);
        }
        glEnd();
        glColor4ub((unsigned char)(fR + 22), (unsigned char)(fG + 30), (unsigned char)(fB + 14), 135);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(tx + tH * 0.02f, podBot - tH * 0.50f + tH * 1.15f);
        for (int k = 0; k <= 10; k++)
        {
            float a = k * 2.f * PI / 10.f;
            glVertex2f(tx + tH * 0.02f + cosf(a) * tH * 0.22f * asp,
                       podBot - tH * 0.50f + tH * 0.98f + sinf(a) * tH * 0.22f);
        }
        glEnd();
    }

    // ----- ANTENNA -----
    float antBase = cy + sry, antTop = antBase + 0.072f;
    glColor3ub(72, 75, 82);
    glLineWidth(2.2f);
    glBegin(GL_LINES);
    glVertex2f(cx, antBase);
    glVertex2f(cx, antTop);
    glEnd();
    glColor3ub(62, 65, 72);
    glLineWidth(1.1f);
    float barsH[] = {0.20f, 0.50f, 0.75f}, barsW[] = {0.022f, 0.014f, 0.008f};
    for (int ab = 0; ab < 3; ab++)
    {
        float aY = antBase + (antTop - antBase) * barsH[ab];
        glBegin(GL_LINES);
        glVertex2f(cx - barsW[ab], aY);
        glVertex2f(cx + barsW[ab], aY);
        glEnd();
    }
    diskFan(cx, antTop, 0.0042f, 0.0042f, 12, 215, 38, 28);
    glColor4ub(255, 72, 55, 88);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, antTop);
    for (int k = 0; k <= 12; k++)
    {
        float a = k * 2.f * PI / 12.f;
        glVertex2f(cx + cosf(a) * 0.0078f, antTop + sinf(a) * 0.0078f);
    }
    glEnd();
}

// ================================================================
//  LEFT BACKGROUND BUILDINGS                                [Emad]
//
//  Five grey high-rise buildings in the background on the left
//  side of the scene, behind the C-Building.  Like Annex-9, the
//  whole row is first laid out in local coordinates (each
//  drawBuilding call uses simple numbers in the -1..+1 range)
//  and then glScalef + glTranslatef shrink and shift the whole
//  row into place.
//
//  drawBuilding(left, bottom, right, top, cols, rows, R, G, B)
//    cols / rows : how many windows on each face
//    R, G, B     : grey shade of the wall
// ================================================================
static void drawLeftBackgroundBuildings()
{
    glPushMatrix();
    glTranslatef(-0.733f, 0.098f, 0.0f); // move row into place
    glScalef(0.282f, 0.48f, 1.0f);       // shrink local design

    // Five towers, drawn from right (shortest) to left (tallest).
    // Each row of drawBuilding() arguments:
    //    left, bottom, right, top, windowCols, windowRows, R, G, B
    drawBuilding(0.44f, -0.83f, 0.75f, 0.28f, 2, 5, 82, 87, 98);
    drawBuilding(0.15f, -0.83f, 0.52f, 0.50f, 3, 7, 76, 81, 92);
    drawBuilding(-0.16f, -0.83f, 0.22f, 0.42f, 3, 6, 80, 85, 96);
    drawBuilding(-0.52f, -0.83f, -0.08f, 0.68f, 3, 9, 74, 79, 90);
    drawBuilding(-0.94f, -0.83f, -0.44f, 0.95f, 4, 12, 78, 83, 95);

    glPopMatrix();
}

// ================================================================
//  RIGHT BACKGROUND BUILDINGS
// ================================================================
static void drawRightBackgroundBuildings()
{
    glPushMatrix();
    glTranslatef(0.30f, -0.051f, 0.0f);
    glScalef(0.78f, 1.25f, 1.0f);
    glColor3f(0.4f, 0.5f, 0.7f);
    glBegin(GL_POLYGON);
    glVertex2f(0.56f, 0.5f);
    glVertex2f(0.56f, -0.2f);
    glVertex2f(0.66f, -0.2f);
    glVertex2f(0.66f, 0.5f);
    glEnd();
    glColor3f(0.9f, 0.95f, 1.0f);
    float xS = 0.57f, yS = 0.47f;
    for (int row = 0; row < 6; row++)
        for (int col = 0; col < 3; col++)
        {
            glBegin(GL_POLYGON);
            glVertex2f(xS + col * 0.03f, yS - row * 0.11f);
            glVertex2f(xS + col * 0.03f, yS - row * 0.11f - 0.05f);
            glVertex2f(xS + col * 0.03f + 0.02f, yS - row * 0.11f - 0.05f);
            glVertex2f(xS + col * 0.03f + 0.02f, yS - row * 0.11f);
            glEnd();
        }
    glColor3f(0.85f, 0.85f, 0.85f);
    glBegin(GL_POLYGON);
    glVertex2f(0.68f, 0.38f);
    glVertex2f(0.68f, -0.18f);
    glVertex2f(0.81f, -0.18f);
    glVertex2f(0.81f, 0.38f);
    glEnd();
    glColor3f(0.6f, 0.2f, 0.2f);
    glBegin(GL_POLYGON);
    glVertex2f(0.685f, 0.37f);
    glVertex2f(0.685f, -0.17f);
    glVertex2f(0.72f, -0.17f);
    glVertex2f(0.72f, 0.37f);
    glEnd();
    glColor3f(0.9f, 0.95f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0.72f, 0.37f);
    glVertex2f(0.72f, -0.17f);
    glVertex2f(0.74f, -0.17f);
    glVertex2f(0.74f, 0.37f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(0.77f, 0.37f);
    glVertex2f(0.77f, -0.17f);
    glVertex2f(0.80f, -0.17f);
    glVertex2f(0.80f, 0.37f);
    glEnd();
    float spandY[] = {0.29f, 0.18f, 0.07f, -0.06f}, spandH[] = {0.25f, 0.14f, 0.03f, -0.10f};
    for (int i = 0; i < 4; i++)
    {
        glColor3f(0.85f, 0.85f, 0.85f);
        glBegin(GL_POLYGON);
        glVertex2f(0.68f, spandY[i]);
        glVertex2f(0.68f, spandH[i]);
        glVertex2f(0.81f, spandH[i]);
        glVertex2f(0.81f, spandY[i]);
        glEnd();
    }
    glColor3f(0.4f, 0.5f, 0.7f);
    glBegin(GL_POLYGON);
    glVertex2f(0.84f, 0.42f);
    glVertex2f(0.84f, -0.215f);
    glVertex2f(1.00f, -0.215f);
    glVertex2f(1.00f, 0.42f);
    glEnd();
    glColor3f(0.9f, 0.95f, 1.0f);
    xS = 0.855f;
    yS = 0.40f;
    for (int row = 0; row < 6; row++)
        for (int col = 0; col < 4; col++)
        {
            glBegin(GL_POLYGON);
            glVertex2f(xS + col * 0.035f, yS - row * 0.11f);
            glVertex2f(xS + col * 0.035f, yS - row * 0.11f - 0.05f);
            glVertex2f(xS + col * 0.035f + 0.025f, yS - row * 0.11f - 0.05f);
            glVertex2f(xS + col * 0.035f + 0.025f, yS - row * 0.11f);
            glEnd();
        }
    glPopMatrix();
}

// ================================================================
//  D-BUILDING //[Prottoy]
// ================================================================
static void drawDBuilding()
{
    // ------------------------------------------------------------------
    //  D‑BUILDING  — main academic block (10 storeys)
    // ------------------------------------------------------------------

    // ----- Building coordinate system -----
    const float bx = -0.15f, by = -0.36f;      // bottom‑left anchor point
    const float bw = 0.90f, bh = 0.88f;        // width and overall height
    const int NF = 10;                         // number of floors
    const float fH = bh / (float)NF;           // single floor height
    const float glassRight = bx + bw * 0.265f; // X boundary between left glass and right concrete
    const float chamH = bh * 0.168f;           // height of the top chamfer on the glass face

    // ----- Right annex (narrow dark tower attached on the right) -----
    drawRightAnnex(bx + bw, by, bh, fH, NF);

    // ================================================================
    //  1. MAIN CONCRETE
    // ================================================================
    // Base solid colour (warm grey)
    solidQ(glassRight, by, bx + bw, by + bh, 202, 203, 198);
    // Subtle warm overlay for depth
    alphaQ(glassRight, by, bx + bw, by + bh, 255, 248, 238, 14); // main body of the building created

    // Warm‑to‑cool vertical gradient overlay on the concrete face
    // (warmer near the bottom, slightly cooler near the top)
    glColor4ub(255, 245, 230, 10);
    glBegin(GL_QUADS);
    glVertex2f(glassRight, by);
    glVertex2f(bx + bw, by);
    glVertex2f(bx + bw, by + bh * 0.5f);
    glVertex2f(glassRight, by + bh * 0.5f);
    glEnd();
    glColor4ub(215, 230, 250, 8);
    glBegin(GL_QUADS);
    glVertex2f(glassRight, by + bh * 0.5f);
    glVertex2f(bx + bw, by + bh * 0.5f);
    glVertex2f(bx + bw, by + bh);
    glVertex2f(glassRight, by + bh);
    glEnd();

    // ================================================================
    //  2. LEFT CURVED GLASS CURTAIN WALL
    // ================================================================
    // Dark blue tinted glass polygon — main shape including the chamfered top
    glColor3ub(42, 65, 132);
    glBegin(GL_POLYGON);
    glVertex2f(bx, by);                      // bottom-left
    glVertex2f(glassRight, by);              // bottom-right on glass
    glVertex2f(glassRight, by + bh - chamH); // start of chamfer
    // Chamfered top edge — smoothly curved
    for (int c = 0; c <= 40; c++)
    {
        float t = c / 40.f;
        float px = glassRight + t * (bx + bw * 0.030f - glassRight);
        float py = (by + bh - chamH) + t * chamH;
        float dip = 0.020f * sinf(t * PI);
        glVertex2f(px + dip, py);
    }
    glVertex2f(bx + bw * 0.030f, by + bh); // top of chamfer
    glVertex2f(bx, by + bh);               // top-left
    glEnd();

    // ----- Glass reflections: sky reflection on upper half -----
    glColor4ub(105, 150, 215, 60);
    glBegin(GL_POLYGON);
    glVertex2f(bx, by + bh * 0.35f);
    glVertex2f(glassRight, by + bh * 0.35f);
    glVertex2f(glassRight, by + bh - chamH); // upper half of the left curtatin reflection
    // Follow the chamfer curve for the reflection boundary
    for (int c = 0; c <= 20; c++)
    {
        float t = c / 20.f;
        float px = glassRight + t * (bx + bw * 0.030f - glassRight); // tried to show a refection effect but quite didn;t worked
        float py = (by + bh - chamH) + t * chamH;
        glVertex2f(px, py);
    }
    glVertex2f(bx + bw * 0.030f, by + bh);
    glVertex2f(bx, by + bh * 0.88f);
    glEnd();

    // ----- Lower reflection (darker, ground/environment bounce) -----
    alphaQ(bx, by, glassRight, by + bh * 0.35f, 45, 70, 118, 32); // tried to show the reflection

    // ----- Bright specular streaks (sun reflection) -----
    // Primary bright streak
    glColor4ub(220, 238, 255, 36);
    glBegin(GL_POLYGON); // bigger vertical quad to show sun reflection
    glVertex2f(bx + 0.010f, by + bh * 0.08f);
    glVertex2f(bx + (glassRight - bx) * 0.42f, by + bh * 0.08f);
    glVertex2f(bx + (glassRight - bx) * 0.36f, by + bh * 0.92f);
    glVertex2f(bx + 0.010f, by + bh * 0.92f);
    glEnd();
    // Secondary narrower streak
    glColor4ub(200, 225, 255, 14); // second quad to show sun reflection
    glBegin(GL_POLYGON);
    glVertex2f(bx + (glassRight - bx) * 0.48f, by + bh * 0.10f);
    glVertex2f(bx + (glassRight - bx) * 0.62f, by + bh * 0.10f);
    glVertex2f(bx + (glassRight - bx) * 0.58f, by + bh * 0.90f);
    glVertex2f(bx + (glassRight - bx) * 0.44f, by + bh * 0.90f);
    glEnd();

    // ================================================================
    //  3. VERTICAL MULLIONS ON THE CURTAIN WALL  (bowed inward)
    // ================================================================
    glColor3ub(26, 36, 55);
    glLineWidth(0.65f);
    int gCols = 30;
    for (int gc = 1; gc < gCols; gc++)
    {
        float frac = gc / (float)gCols;
        float baseX = bx + frac * (glassRight - bx);
        glBegin(GL_LINE_STRIP);
        for (int s = 0; s <= 56; s++)
        {
            float t = (float)s / 56.f;
            float yy = by + t * bh;
            float bow = 0.019f * sinf(t * PI) * (1.f - frac * 0.55f); // to create the curved lines, horizontal x is shifted at each iteration
            glVertex2f(baseX + bow, yy);
        }
        glEnd();
    }

    // ── NIGHT LIGHTING on left curtain wall (drawn after mullions) ──
    {
        float db = getDayBlend();           // just giving me the dayfactor value.
        float night = smoothStep(1.f - db); // making the day to night and night to day transition like S curve.
        if (night > 0.02f)
        {
            float nf = night * night;         // intesnsified this effect more
            const float leftMargin = 0.020f;  // How far from left edge the light starts
            const float rightMargin = 0.014f; // How far from right edge the light ends
            for (int f = 0; f < NF; f++)
            {
                if (f == 2 || f == 4 || f == 8 || f == 9)
                    continue;
                float fy2 = by + f * fH;
                float wY2 = fy2 + fH * 0.16f; // Top Y of the light band
                float wH2 = fH * 0.62f;       // Height of the light band
                float brightness = 0.75f + 0.25f * ((f * 7) % 5) / 4.0f;
                unsigned char alpha = (unsigned char)(nf * 185.f * brightness);
                glColor4ub(255, 218, 155, alpha);

                // Draw curved polygon matching the bowed curtain wall
                // Bottom edge (left → right with bow), then top edge (right → left with bow)
                int steps = 24;
                glBegin(GL_POLYGON);
                // Bottom curved edge
                for (int s = 0; s <= steps; s++)
                {
                    float frac = leftMargin / (glassRight - bx) +
                                 (float)s / steps *
                                     ((glassRight - rightMargin - bx) - leftMargin) / (glassRight - bx); // Polygon shaped brightness br created on to the left curtain
                    float xBase = bx + frac * (glassRight - bx);
                    float tNorm = wY2 - by; // how far down the building
                    float bow = 0.009f * sinf((tNorm / bh) * PI) * (1.f - frac * 0.155f);
                    glVertex2f(xBase + bow, wY2);
                }
                // Top curved edge (reversed)
                for (int s = steps; s >= 0; s--)
                {
                    float frac = leftMargin / (glassRight - bx) +
                                 (float)s / steps *
                                     ((glassRight - rightMargin - bx) - leftMargin) / (glassRight - bx);
                    float xBase = bx + frac * (glassRight - bx);
                    float tNorm = wY2 + wH2 - by;
                    float bow = 0.009f * sinf((tNorm / bh) * PI) * (1.f - frac * 0.55f);
                    glVertex2f(xBase + bow, wY2 + wH2);
                }
                glEnd();
            }
        }
    }

    // ================================================================
    //  4. CHAMFER GLASS PANEL  (angled top portion of the glass face)
    // ================================================================
    glColor4ub(52, 78, 118, 220); // top of the left certain tried it to look like sun  reflection.
    glBegin(GL_POLYGON);
    glVertex2f(glassRight, by + bh - chamH);
    glVertex2f(glassRight, by + bh);
    glVertex2f(bx + bw * 0.030f, by + bh);
    glVertex2f(bx, by + bh);
    glVertex2f(bx, by + bh - chamH * 0.35f);
    glEnd();

    // Diagonal grid lines on the chamfer panel
    glColor4ub(28, 40, 60, 165); // the lines on the top of left chamber
    glLineWidth(0.55f);
    for (int c = 1; c <= 9; c++)
    {
        float t = c / 10.f;
        float px = glassRight + t * (bx + bw * 0.030f - glassRight);
        float py = (by + bh - chamH) + t * chamH;
        glBegin(GL_LINES);
        glVertex2f(glassRight, py);
        glVertex2f(px, by + bh);
        glEnd();
    }
    for (int c = 1; c <= 6; c++)
    {
        float t = c / 7.f;
        float gy = (by + bh - chamH) + t * chamH;
        float gx_end = glassRight + t * (bx + bw * 0.030f - glassRight);
        glBegin(GL_LINES);
        glVertex2f(bx, gy);
        glVertex2f(gx_end, by + bh);
        glEnd();
    }

    // ================================================================
    //  5. VERTICAL STONE PANEL JOINTS  (on the concrete facade)
    // ================================================================
    glColor4ub(165, 167, 163, 100);
    glLineWidth(0.55f);
    int nPanels = 34;
    float panelW = (bx + bw - glassRight) / (float)nPanels;
    for (int p = 1; p < nPanels; p++)
    {
        float px = glassRight + p * panelW;
        glBegin(GL_LINES);
        glVertex2f(px, by);
        glVertex2f(px, by + bh);
        glEnd();
    }

    // Subtle horizontal texture lines on the concrete panels
    glColor4ub(185, 187, 183, 40);
    glLineWidth(0.35f);
    for (int p = 0; p < nPanels * 3; p++)
    {
        float py = by + p * (bh / (nPanels * 3.f));
        glBegin(GL_LINES);
        glVertex2f(glassRight, py);
        glVertex2f(bx + bw, py);
        glEnd();
    }

    // ================================================================
    //  6. PER‑FLOOR WINDOW DETAILS
    // ================================================================
    for (int f = 0; f < NF; f++)
    {
        float fy = by + f * fH; // bottom Y of current floor

        // Horizontal floor‑separator band (concrete spandrel)
        solidQ(glassRight, fy, bx + bw, fy + 0.0095f, 180, 182, 177);
        alphaQ(glassRight, fy + 0.0095f, bx + bw, fy + 0.013f, 255, 255, 255, 16);

        // Window area dimensions for this floor
        float wX = glassRight + 0.014f;
        float wW = bx + bw - 0.010f - wX;
        float wY = fy + fH * 0.14f;
        float wH = fH * 0.67f;

        // ---- LOUNGE FLOORS (4th and 7th, zero‑based f==3 and f==6) ----
        if (f == 3 || f == 6)
        {
            // Dark recessed background
            solidQ(wX - 0.010f, fy + fH * 0.02f, wX + wW + 0.010f, fy + fH * 0.57f, 28, 32, 38);
            // Translucent glass pane
            glColor4ub(58, 88, 138, 200);
            glBegin(GL_QUADS);
            glVertex2f(wX, fy + fH * 0.03f);
            glVertex2f(wX + wW, fy + fH * 0.03f);
            glVertex2f(wX + wW, fy + fH * 0.56f);
            glVertex2f(wX, fy + fH * 0.56f);
            glEnd();
            // Reflection on the lounge glass
            alphaQ(wX, fy + fH * 0.03f, wX + wW * 0.40f, fy + fH * 0.56f, 180, 210, 255, 14);

            // Vertical mullions on the lounge glass
            glColor3ub(26, 32, 42);
            glLineWidth(0.88f);
            for (int d = 1; d <= 20; d++)
            {
                float mx = wX + d * wW / 21.f;
                glBegin(GL_LINES);
                glVertex2f(mx, fy + fH * 0.03f);
                glVertex2f(mx, fy + fH * 0.56f);
                glEnd();
            }
            // Horizontal mid‑rail
            glBegin(GL_LINES);
            glVertex2f(wX, fy + fH * 0.295f);
            glVertex2f(wX + wW, fy + fH * 0.295f);
            glEnd();

            // Concrete ledge at the bottom of the terrace
            solidQ(wX - 0.024f, fy + fH * 0.53f, wX + wW + 0.024f, fy + fH * 0.61f, 184, 187, 182);
            alphaQ(wX - 0.024f, fy + fH * 0.53f, wX + wW + 0.024f, fy + fH * 0.558f, 0, 0, 0, 38);

            // Vegetation / terrace greenery
            drawLoungeTerrace(wX - 0.00f, fy + fH * 0.00f, wW + 0.000f, fH * 0.34f, fH * 0.14f); // user-defined function to create the terrance on to the lounge

            // ---- TOP FLOOR (10th) — louvred sun‑screen ----
        }
        else if (f == NF - 1)
        {
            solidQ(wX - 0.008f, wY, wX + wW + 0.008f, wY + wH, 36, 40, 46); // dark frame
            glColor3ub(52, 58, 66);
            glLineWidth(0.72f);
            int nLouv = 16; // 16 horizontal line on to the top floor.
            for (int lv = 1; lv < nLouv; lv++)
            {
                float ly = wY + lv * wH / (float)nLouv;
                glBegin(GL_LINES);
                glVertex2f(wX, ly);
                glVertex2f(wX + wW, ly);
                glEnd();
            }
            // Vertical dividers
            for (int d = 1; d < 14; d++)
            { // 14 vertical lines
                float dx = wX + d * wW / 14.f;
                glBegin(GL_LINES);
                glVertex2f(dx, wY);
                glVertex2f(dx, wY + wH);
                glEnd();
            }

            // ---- REGULAR FLOORS (1‑3, 5, 8‑9) ----
        }
        else
        {
            // Dark window frame
            solidQ(wX - 0.007f, wY - 0.005f, wX + wW + 0.007f, wY + wH + 0.005f, 50, 55, 62); // dark frame for the window floors
            // Glazing — lit/unlit pattern
            bool lit = ((f * 3 + 1) % 4 != 0);
            glColor3ub(lit ? 70 : 52, lit ? 102 : 76, lit ? 150 : 112);
            glBegin(GL_QUADS);
            glVertex2f(wX, wY);
            glVertex2f(wX + wW, wY);
            glVertex2f(wX + wW, wY + wH);
            glVertex2f(wX, wY + wH);
            glEnd();
            // Soft reflection on the glass
            glColor4ub(185, 215, 255, 16);
            glBegin(GL_QUADS);
            glVertex2f(wX + 0.004f, wY);
            glVertex2f(wX + wW * 0.40f, wY);
            glVertex2f(wX + wW * 0.36f, wY + wH);
            glVertex2f(wX + 0.004f, wY + wH); // tried to create reflection on to the glass
            glEnd();
            // Horizontal transom (mid‑rail)
            solidQ(wX, wY + wH * 0.47f, wX + wW, wY + wH * 0.515f, 46, 51, 58); // tried to create horizontal lines
            // Vertical mullions
            glColor3ub(44, 49, 56);
            glLineWidth(0.70f); // the vertical lines of the windows
            int nDiv = 26;
            for (int d = 1; d < nDiv; d++)
            {
                float dx = wX + d * wW / (float)nDiv;
                glBegin(GL_LINES);
                glVertex2f(dx, wY);
                glVertex2f(dx, wY + wH);
                glEnd();
            }
            // Window sill (bottom ledge)
            solidQ(wX - 0.010f, wY - 0.005f, wX + wW + 0.010f, wY, 180, 182, 177); // bottom edge of the windows
        }
    }

    // ================================================================
    //  7. STRUCTURAL EDGE COLUMN  (junction between glass and concrete)
    // ================================================================
    solidQ(glassRight - 0.007f, by, glassRight + 0.018f, by + bh, 168, 170, 165);     // dividers between the left certain and floor glass
    alphaQ(glassRight - 0.002f, by, glassRight + 0.006f, by + bh, 255, 255, 255, 26); // dividers between the left certain and floor glass

    // ================================================================
    //  8. GROUND FLOOR — LOBBY & ENTRANCE
    // ================================================================
    float lobX = bx + bw * 0.300f, lobW = bw * 0.380f, lobH = fH * 0.915f;
    // Dark frame behind the glass
    solidQ(lobX - 0.012f, by, lobX + lobW + 0.012f, by + lobH, 22, 25, 30);
    // Lobby glass
    glColor4ub(60, 95, 150, 218);
    glBegin(GL_QUADS);
    glVertex2f(lobX, by);
    glVertex2f(lobX + lobW, by);
    glVertex2f(lobX + lobW, by + lobH);
    glVertex2f(lobX, by + lobH);
    glEnd();
    // Glass reflection
    alphaQ(lobX, by + lobH * 0.28f, lobX + lobW * 0.42f, by + lobH, 210, 230, 255, 16);

    // Lobby vertical mullions
    glColor3ub(30, 38, 50); // entarance gate bars
    glLineWidth(1.05f);
    for (int d = 1; d <= 5; d++)
    {
        float dx = lobX + d * lobW / 6.f;
        glBegin(GL_LINES);
        glVertex2f(dx, by);
        glVertex2f(dx, by + lobH);
        glEnd();
    }
    glBegin(GL_LINES);
    glVertex2f(lobX, by + lobH * 0.62f);
    glVertex2f(lobX + lobW, by + lobH * 0.62f);
    glEnd();

    // Entry door recesses (three darker panels)
    for (int d = 0; d < 3; d++)
    {
        float dx = lobX + (d + 0.5f) * lobW / 3.f, dw = lobW * 0.12f;
        solidQ(dx - dw * 0.5f, by, dx + dw * 0.5f, by + lobH * 0.58f, 18, 20, 26);
        glColor4ub(80, 120, 180, 120);
        glBegin(GL_QUADS);
        glVertex2f(dx - dw * 0.5f + 0.004f, by + 0.004f);
        glVertex2f(dx + dw * 0.5f - 0.004f, by + 0.004f);
        glVertex2f(dx + dw * 0.5f - 0.004f, by + lobH * 0.56f);
        glVertex2f(dx - dw * 0.5f + 0.004f, by + lobH * 0.56f); // entry doors
        glEnd();
    }

    // Ground‑floor columns (left, centre, right)
    float colW2 = 0.022f;
    solidQ(glassRight - 0.005f, by, glassRight + colW2, by + lobH + 0.014f, 188, 190, 186);
    float cenX = lobX + lobW * 0.5f - colW2 * 0.5f;
    solidQ(cenX, by, cenX + colW2, by + lobH + 0.014f, 188, 190, 186);
    solidQ(bx + bw - colW2, by, bx + bw + 0.006f, by + lobH + 0.014f, 188, 190, 186);
    // Column caps
    for (int c = 0; c < 3; c++)
    {
        float cxCol = (c == 0) ? glassRight - 0.005f : ((c == 1) ? cenX : bx + bw - colW2);
        solidQ(cxCol - 0.005f, by + lobH + 0.010f, cxCol + colW2 + 0.012f, by + lobH + 0.022f, 175, 177, 173);
    }

    // Red accent band above entry
    solidQ(lobX - 0.022f, by + lobH * 0.695f, lobX + lobW + 0.022f, by + lobH * 0.695f + 0.013f, 192, 20, 14);
    // Canopy / overhang
    solidQ(lobX - 0.032f, by + lobH * 0.90f, lobX + lobW + 0.032f, by + lobH * 0.90f + 0.010f, 165, 168, 162);
    alphaQ(lobX - 0.032f, by + lobH * 0.90f + 0.010f, lobX + lobW + 0.032f, by + lobH * 0.90f + 0.015f, 0, 0, 0, 32);

    // ================================================================
    //  9. ROOFTOP
    // ================================================================
    float roofY = by + bh;

    // Green roof garden strip
    solidQ(bx, roofY, bx + bw, roofY + 0.032f, 36, 100, 30);
    // Parapet wall
    solidQ(bx, roofY + 0.028f, bx + bw, roofY + 0.044f, 192, 194, 190);
    solidQ(bx, roofY + 0.040f, bx + bw, roofY + 0.048f, 178, 180, 175);

    // Red accent panels on roofline
    solidQ(bx + bw * 0.18f, roofY, bx + bw * 0.44f, roofY + 0.044f, 188, 22, 16);
    solidQ(bx + bw * 0.56f, roofY, bx + bw * 0.82f, roofY + 0.044f, 188, 22, 16);

    // Left canopy structure (solar/shade slab)
    float canX = bx - 0.018f, canW = bw * 0.50f, canBase = roofY + 0.044f, canColH = 0.064f;
    float cPos[] = {0.08f, 0.24f, 0.40f};
    for (int c = 0; c < 3; c++)
    {
        float cxC = canX + cPos[c] * canW;
        solidQ(cxC - 0.010f, canBase, cxC + 0.010f, canBase + canColH, 120, 124, 130);
    }
    // Canopy slab
    solidQ(canX - 0.024f, canBase + canColH, canX + canW + 0.024f, canBase + canColH + 0.020f, 44, 48, 52);
    solidQ(canX - 0.018f, canBase + canColH, canX + canW + 0.018f, canBase + canColH + 0.007f, 66, 70, 74);
    // Canopy top with solar panel detailing
    solidQ(canX, canBase + canColH + 0.005f, canX + canW, canBase + canColH + 0.014f, 24, 31, 48);
    glColor4ub(40, 56, 88, 190);
    glLineWidth(0.52f);
    for (int s = 1; s <= 15; s++)
    {
        float sx = canX + s * canW / 16.f;
        glBegin(GL_LINES);
        glVertex2f(sx, canBase + canColH + 0.005f);
        glVertex2f(sx, canBase + canColH + 0.014f);
        glEnd();
    }
    for (int r = 1; r < 3; r++)
    {
        float sy = canBase + canColH + 0.005f + r * (0.009f / 3.f);
        glBegin(GL_LINES);
        glVertex2f(canX, sy);
        glVertex2f(canX + canW, sy);
        glEnd();
    }

    // Right pergola / open‑frame structure
    float pX = bx + bw * 0.52f, pW = bw * 0.43f, pBase = roofY + 0.044f, pH = 0.096f;
    // Green base inside pergola
    solidQ(pX, pBase + 0.008f, pX + pW, pBase + 0.040f, 32, 105, 28);
    // Small vegetation tufts inside pergola
    glColor3ub(44, 148, 40);
    for (int t = 0; t < 8; t++)
    {
        float tx = pX + (t + 0.5f) * pW / 8.f, th2 = 0.044f + 0.024f * (t % 3) / 2.f;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(tx, pBase + 0.008f + th2);
        for (int k = 0; k <= 14; k++)
        {
            float a = k * 2.f * PI / 14.f;
            glVertex2f(tx + cosf(a) * 0.022f, pBase + 0.008f + th2 * 0.5f + sinf(a) * th2 * 0.50f);
        }
        glEnd();
    }
    // Pergola columns
    float pCP[] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
    for (int c = 0; c < 5; c++)
    {
        float cxP = pX + pCP[c] * pW;
        solidQ(cxP - 0.009f, pBase, cxP + 0.009f, pBase + pH, 163, 166, 162);
    }
    // Pergola top beam
    solidQ(pX - 0.012f, pBase + pH, pX + pW + 0.012f, pBase + pH + 0.018f, 48, 52, 56);
    glColor3ub(40, 44, 48);
    glLineWidth(2.0f);
    for (int b = 0; b <= 6; b++)
    {
        float bx2 = pX + b * pW / 6.f;
        glBegin(GL_LINES);
        glVertex2f(bx2, pBase + pH);
        glVertex2f(bx2, pBase + pH + 0.018f);
        glEnd();
    }

    // ================================================================
    // 10. AIUB SIGN  (rooftop lettering)
    // ================================================================
    drawAIUB(bx + bw * 0.368f, roofY + 0.054f, bw * 0.264f, 0.042f);

    // Flag poles on rooftop
    glColor3ub(96, 98, 104);
    glLineWidth(2.4f);
    glBegin(GL_LINES);
    glVertex2f(bx + bw * 0.295f, roofY + 0.044f + canColH + 0.020f);
    glVertex2f(bx + bw * 0.295f, roofY + 0.044f + canColH + 0.020f + 0.078f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(bx + bw * 0.700f, pBase + pH + 0.018f);
    glVertex2f(bx + bw * 0.700f, pBase + pH + 0.018f + 0.062f);
    glEnd();
    // Flag ornaments (red dots)
    diskFan(bx + bw * 0.295f, roofY + 0.044f + canColH + 0.020f + 0.078f, 0.0060f, 0.0060f, 12, 198, 28, 22); // anteenas of D-building
    diskFan(bx + bw * 0.700f, pBase + pH + 0.018f + 0.062f, 0.0052f, 0.0052f, 12, 198, 28, 22);

    // ================================================================
    // 11. BUILDING OUTLINE  (final clean silhouette)
    // ================================================================
    // Main outline (right + bottom edges)
    glColor3ub(28, 34, 46);
    glLineWidth(2.2f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(bx, by);
    glVertex2f(bx + bw, by);
    glVertex2f(bx + bw, by + bh);
    glVertex2f(glassRight, by + bh);
    glEnd();
    // Softer outline for the left glass face
    glColor3ub(36, 50, 168);
    glLineWidth(1.70f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(bx, by);
    glVertex2f(bx, by + bh); // a outline around left certain
    glVertex2f(bx + bw * 0.030f, by + bh);
    glEnd();
}

// ================================================================
//  ROAD, FOOTBALL FIELD, BASKETBALL COURT [Prottoy, EMAD, Shajmin]
// ================================================================
static void drawRoadAndPlayground()
{
    float asp = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
    float X1 = -asp, X2 = asp;

    // ================================================================
    //  ROAD SURFACE
    // ================================================================
    // Main asphalt base
    solidQ(X1, -1.00f, X2, -0.72f, 52, 52, 52);
    // Subtle lighter overlay for asphalt depth
    alphaQ(X1, -0.99f, X2, -0.73f, 80, 80, 80, 40);

    float roadY1 = -1.00f, roadY2 = -0.72f;

    // Road edge white lines
    glColor3ub(240, 240, 240);
    glLineWidth(1.8f);
    glBegin(GL_LINES);
    glVertex2f(X1, roadY2 - 0.006f);
    glVertex2f(X2, roadY2 - 0.006f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(X1, roadY1 + 0.010f);
    glVertex2f(X2, roadY1 + 0.010f);
    glEnd();

    // Kerb strip
    solidQ(X1, roadY2 - 0.006f, X2, roadY2 + 0.012f, 178, 178, 175);

    // Centre dashed yellow dividing line
    glColor3ub(255, 220, 50);
    glLineWidth(2.5f);
    float roadWidth = X2 - X1;
    int nDashes = 18;
    for (int d = 0; d < nDashes; d++)
    {
        float dashX1 = X1 + d * (roadWidth / nDashes);
        float dashX2 = dashX1 + (roadWidth / nDashes) * 0.55f;
        float lineY = (roadY1 + roadY2) * 0.5f;
        glBegin(GL_LINES);
        glVertex2f(dashX1, lineY);
        glVertex2f(dashX2, lineY);
        glEnd();
    }

    // ================================================================
    //  SHARED Y EXTENTS FOR ALL THREE ZONES
    // ================================================================
    float fieldY1 = roadY2 + 0.012f;
    float fieldY2 = fieldY1 + 0.335f;
    float zoneH = fieldY2 - fieldY1;
    float zoneCY = (fieldY1 + fieldY2) * 0.5f;

    // ================================================================
    //  ZONE 1 — Parking lot, parked cars, security booth, lamps
    // ================================================================
    drawParkingLot();          // [Emad]    asphalt floor, bay lines, dashed lane, border
    drawParkedCars();          // [Shajmin] static parked cars filling the bays
    drawParkingExitBuilding(); // [Shajmin] security booth at the left side of the lot
    drawParkingArm();          // [Shajmin] rotating boom barrier (dynamic rotation)

    // ── LAMP POSTS IN PARKING LOT ────────────────────────────────
    {
        const float lotX1 = -0.880f;
        const float laneTopY = -0.486f; // top of drive lane = base of top bay row

        float lampXs[] = {lotX1 + 0.20f, lotX1 + 0.45f, lotX1 + 0.70f};
        float lampBaseY = laneTopY;
        for (int lp = 0; lp < 3; lp++)
        {
            float lx = lampXs[lp];
            float ly = lampBaseY;
            float postH = 0.130f;

            // Base slab
            solidQ(lx - 0.012f, ly, lx + 0.012f, ly + 0.008f, 80, 76, 68);
            // Post
            solidQ(lx - 0.005f, ly + 0.007f, lx + 0.005f, ly + postH, 68, 64, 56);
            // Lamp head arm
            glColor3ub(60, 57, 50);
            glLineWidth(2.2f);
            glBegin(GL_LINE_STRIP);
            glVertex2f(lx, ly + postH);
            glVertex2f(lx + 0.018f, ly + postH + 0.010f);
            glVertex2f(lx + 0.030f, ly + postH + 0.014f);
            glEnd();
            // Lamp housing
            solidQ(lx + 0.018f, ly + postH + 0.010f, lx + 0.044f, ly + postH + 0.018f, 48, 46, 38);
            // Glow
            glColor4ub(255, 240, 150, 80);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(lx + 0.031f, ly + postH + 0.012f);
            for (int k = 0; k <= 16; k++)
            {
                float a = k * 2.f * PI / 16.f;
                glColor4ub(255, 228, 120, 0);
                glVertex2f(lx + 0.031f + cosf(a) * 0.040f, ly + postH + 0.012f + sinf(a) * 0.030f);
            }
            glEnd();
            // Lamp lens
            diskFan(lx + 0.031f, ly + postH + 0.014f, 0.010f, 0.005f, 10, 255, 252, 200);
        }
    }

    // ================================================================
    //  ZONE 2 — BASKETBALL COURT (centre-left)
    // ================================================================
    float bktX1 = -0.10f, bktX2 = 0.35f;
    float bktW = bktX2 - bktX1;
    float bktH = zoneH;
    float bktCX = (bktX1 + bktX2) * 0.5f;
    float bm = 0.016f; // boundary margin

    // ----------------------------------------------------------------
    //  Hardwood floor — alternating plank stripes (horizontal grain)
    // ----------------------------------------------------------------
    int nPlanks = 10;
    for (int p = 0; p < nPlanks; p++)
    {
        float py1 = fieldY1 + p * bktH / nPlanks;
        float py2 = py1 + bktH / nPlanks;
        unsigned char shade = (p % 2 == 0) ? 195 : 180;
        solidQ(bktX1, py1, bktX2, py2, shade, (unsigned char)(shade - 42), 78); // basketball court stripes
    }

    // ----------------------------------------------------------------
    //  Court outer boundary
    // ----------------------------------------------------------------
    glColor3ub(255, 255, 255);
    glLineWidth(1.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bktX1 + bm, fieldY1 + bm);
    glVertex2f(bktX2 - bm, fieldY1 + bm);
    glVertex2f(bktX2 - bm, fieldY2 - bm);
    glVertex2f(bktX1 + bm, fieldY2 - bm);
    glEnd();

    // ----------------------------------------------------------------
    //  Half-court centre line (vertical — splits left/right halves)
    // ----------------------------------------------------------------
    glBegin(GL_LINES);
    glVertex2f(bktCX, fieldY1 + bm);
    glVertex2f(bktCX, fieldY2 - bm);
    glEnd();

    // ----------------------------------------------------------------
    //  Centre jump-ball circle — radius fits within court height
    // ----------------------------------------------------------------
    float jbR = bktH * 0.16f; // jump ball radius
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 48; i++)
    {
        float a = i * 2.f * PI / 48.f;
        // divide X by asp to keep it a true circle on screen
        glVertex2f(bktCX + cosf(a) * jbR / asp, zoneCY + sinf(a) * jbR);
    }
    glEnd();

    // Centre dot
    diskFan(bktCX, zoneCY, 0.005f / asp, 0.005f, 14, 255, 255, 255);

    // ----------------------------------------------------------------
    //  LEFT KEY (paint) — left end, opens toward centre
    // ----------------------------------------------------------------
    float keyW = bktW * 0.28f; // depth of key (horizontal)
    float keyH = bktH * 0.44f; // height of key box
    float keyY1 = zoneCY - keyH * 0.5f;
    float keyY2 = zoneCY + keyH * 0.5f;
    float keyRX = bktX1 + bm;    // left edge of key (at boundary)
    float keyRX2 = keyRX + keyW; // right edge of key box

    // Paint fill
    solidQ(keyRX, keyY1, keyRX2, keyY2, 180, 80, 40);
    alphaQ(keyRX, keyY1, keyRX2, keyY2, 255, 110, 60, 50);

    // Key outline
    glColor3ub(255, 255, 255);
    glLineWidth(1.4f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(keyRX, keyY1);
    glVertex2f(keyRX2, keyY1);
    glVertex2f(keyRX2, keyY2);
    glVertex2f(keyRX, keyY2);
    glEnd();

    // Restricted area arc (inside key, semi-circle at basket end)
    float raR = keyH * 0.22f;
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++)
    {
        float a = -PI * 0.5f + (float)i / 24.f * PI; // right-facing semicircle
        float px = keyRX + cosf(a) * raR / asp;
        float py = zoneCY + sinf(a) * raR;
        if (px >= keyRX)
            glVertex2f(px, py);
    }
    glEnd();

    // ----------------------------------------------------------------
    //  LEFT FREE-THROW line and circle
    // ----------------------------------------------------------------
    // Free-throw line (vertical line at far end of key)
    glBegin(GL_LINES);
    glVertex2f(keyRX2, keyY1);
    glVertex2f(keyRX2, keyY2);
    glEnd();

    // Free-throw circle centred on the free-throw line
    float ftR = keyH * 0.44f * 0.5f; // same as half key height
    // Solid half (inside key)
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++)
    {
        float a = PI * 0.5f + (float)i / 24.f * PI; // left half
        glVertex2f(keyRX2 + cosf(a) * ftR / asp, zoneCY + sinf(a) * ftR);
    }
    glEnd();
    // Dashed half (outside key)
    glLineWidth(1.0f);
    int ftSegs = 24;
    for (int i = 0; i < ftSegs; i++)
    {
        if (i % 2 == 0)
        { // The reason of dashed circle
            float a1 = -PI * 0.5f + (float)i / ftSegs * PI;
            float a2 = -PI * 0.5f + (float)(i + 1) / ftSegs * PI;
            glBegin(GL_LINES);
            glVertex2f(keyRX2 + cosf(a1) * ftR / asp, zoneCY + sinf(a1) * ftR);
            glVertex2f(keyRX2 + cosf(a2) * ftR / asp, zoneCY + sinf(a2) * ftR);
            glEnd();
        }
    }

    // ----------------------------------------------------------------
    //  LEFT THREE-POINT ARC
    // ----------------------------------------------------------------
    glColor3ub(255, 255, 255);
    glLineWidth(1.6f);
    float tpR = bktH * 0.42f; // three-point radius
    float bsktX = bktX1 + bm; // basket X position (on left boundary)
    float tpY1 = zoneCY - tpR;
    float tpY2 = zoneCY + tpR;
    // Clamp to within court boundary
    if (tpY1 < fieldY1 + bm)
        tpY1 = fieldY1 + bm;
    if (tpY2 > fieldY2 - bm)
        tpY2 = fieldY2 - bm;

    // Straight side lines of three-point zone
    glBegin(GL_LINES);
    glVertex2f(bktX1 + bm, tpY1);
    glVertex2f(bktX1 + bm + bktW * 0.12f, tpY1);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(bktX1 + bm, tpY2);
    glVertex2f(bktX1 + bm + bktW * 0.12f, tpY2);
    glEnd();

    // Arc portion
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 48; i++)
    {
        float a = -PI * 0.5f + (float)i / 48.f * PI; // right-facing arc
        float px = bsktX + cosf(a) * tpR / asp;
        float py = zoneCY + sinf(a) * tpR;
        if (py < fieldY1 + bm || py > fieldY2 - bm)
            continue;
        if (px > bktCX)
            break; // stop at half-court
        glVertex2f(px, py);
    }
    glEnd();

    // ----------------------------------------------------------------
    //  RIGHT KEY (mirror of left)
    // ----------------------------------------------------------------
    float keyLX = bktX2 - bm;    // right boundary
    float keyLX2 = keyLX - keyW; // left edge of right key

    // Paint fill
    solidQ(keyLX2, keyY1, keyLX, keyY2, 180, 80, 40);
    alphaQ(keyLX2, keyY1, keyLX, keyY2, 255, 110, 60, 50);

    // Key outline
    glColor3ub(255, 255, 255);
    glLineWidth(1.4f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(keyLX2, keyY1);
    glVertex2f(keyLX, keyY1);
    glVertex2f(keyLX, keyY2);
    glVertex2f(keyLX2, keyY2);
    glEnd();

    // Restricted area arc (left-facing)
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++)
    {
        float a = PI * 0.5f + (float)i / 24.f * PI; // left-facing semicircle
        float px = keyLX + cosf(a) * raR / asp;
        float py = zoneCY + sinf(a) * raR;
        if (px <= keyLX)
            glVertex2f(px, py);
    }
    glEnd();

    // Right free-throw line
    glBegin(GL_LINES);
    glVertex2f(keyLX2, keyY1);
    glVertex2f(keyLX2, keyY2);
    glEnd();

    // Right free-throw circle solid half
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++)
    {
        float a = -PI * 0.5f + (float)i / 24.f * PI; // right half
        glVertex2f(keyLX2 + cosf(a) * ftR / asp, zoneCY + sinf(a) * ftR);
    }
    glEnd();
    // Dashed half
    for (int i = 0; i < ftSegs; i++)
    {
        if (i % 2 == 0)
        {
            float a1 = PI * 0.5f + (float)i / ftSegs * PI;
            float a2 = PI * 0.5f + (float)(i + 1) / ftSegs * PI;
            glBegin(GL_LINES);
            glVertex2f(keyLX2 + cosf(a1) * ftR / asp, zoneCY + sinf(a1) * ftR);
            glVertex2f(keyLX2 + cosf(a2) * ftR / asp, zoneCY + sinf(a2) * ftR);
            glEnd();
        }
    }

    // ----------------------------------------------------------------
    //  RIGHT THREE-POINT ARC (mirror of left)
    // ----------------------------------------------------------------
    glColor3ub(255, 255, 255);
    glLineWidth(1.6f);
    float bsktRX = bktX2 - bm;

    glBegin(GL_LINES);
    glVertex2f(bktX2 - bm, tpY1);
    glVertex2f(bktX2 - bm - bktW * 0.12f, tpY1);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(bktX2 - bm, tpY2);
    glVertex2f(bktX2 - bm - bktW * 0.12f, tpY2);
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 48; i++)
    {
        float a = PI * 0.5f + (float)i / 48.f * PI; // left-facing arc
        float px = bsktRX + cosf(a) * tpR / asp;
        float py = zoneCY + sinf(a) * tpR;
        if (py < fieldY1 + bm || py > fieldY2 - bm)
            continue;
        if (px < bktCX)
            break;
        glVertex2f(px, py);
    }
    glEnd();

    // ----------------------------------------------------------------
    //  Basketball court border outline
    // ----------------------------------------------------------------
    glColor3ub(140, 100, 40);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bktX1, fieldY1);
    glVertex2f(bktX2, fieldY1);
    glVertex2f(bktX2, fieldY2);
    glVertex2f(bktX1, fieldY2);
    glEnd();

    // ================================================================
    //  ZONE 3 — FOOTBALL (SOCCER) FIELD (right)
    // ================================================================
    float fldX1 = 0.35f, fldX2 = 0.95f;
    float fldW = fldX2 - fldX1;
    float fldCX = (fldX1 + fldX2) * 0.5f;
    float fm = 0.016f;

    // ----------------------------------------------------------------
    //  Alternating mowed turf stripes — VERTICAL bands (left→right)
    // ----------------------------------------------------------------
    int nStripes = 18;
    for (int s = 0; s < nStripes; s++)
    {
        float sx2 = fldX1 + s * fldW / nStripes;
        float ex = sx2 + fldW / nStripes;
        unsigned char g = (s % 2 == 0) ? 118 : 102;
        solidQ(sx2, fieldY1, ex, fieldY2, 38, g, 32);
    }

    // ----------------------------------------------------------------
    //  Outer boundary
    // ----------------------------------------------------------------
    glColor3ub(240, 245, 240);
    glLineWidth(1.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(fldX1 + fm, fieldY1 + fm);
    glVertex2f(fldX2 - fm, fieldY1 + fm);
    glVertex2f(fldX2 - fm, fieldY2 - fm);
    glVertex2f(fldX1 + fm, fieldY2 - fm);
    glEnd();

    // ----------------------------------------------------------------
    //  Half-way line — vertical line splitting left/right halves
    // ----------------------------------------------------------------
    glBegin(GL_LINES);
    glVertex2f(fldCX, fieldY1 + fm);
    glVertex2f(fldCX, fieldY2 - fm);
    glEnd();

    // ----------------------------------------------------------------
    //  Centre circle — aspect-corrected true circle
    // ----------------------------------------------------------------
    float fCR = zoneH * 0.20f;
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= 48; i++)
    {
        float a = i * 2.f * PI / 48.f;
        glVertex2f(fldCX + cosf(a) * fCR / asp, zoneCY + sinf(a) * fCR);
    }
    glEnd();

    // Centre spot
    diskFan(fldCX, zoneCY, 0.005f / asp, 0.005f, 12, 240, 245, 240); // to draw the circle

    // ----------------------------------------------------------------
    //  Penalty areas — left and right ends (horizontal layout)
    // ----------------------------------------------------------------
    float penD = fldW * 0.22f;  // depth of penalty box from side line
    float penH = zoneH * 0.60f; // height of penalty box
    float penY1 = zoneCY - penH * 0.5f;
    float penY2 = zoneCY + penH * 0.5f;

    // Left penalty box (from left boundary inward)
    glBegin(GL_LINE_LOOP);
    glVertex2f(fldX1 + fm, penY1);
    glVertex2f(fldX1 + fm + penD, penY1);
    glVertex2f(fldX1 + fm + penD, penY2);
    glVertex2f(fldX1 + fm, penY2);
    glEnd();

    // Right penalty box (from right boundary inward)
    glBegin(GL_LINE_LOOP);
    glVertex2f(fldX2 - fm, penY1);
    glVertex2f(fldX2 - fm - penD, penY1);
    glVertex2f(fldX2 - fm - penD, penY2);
    glVertex2f(fldX2 - fm, penY2);
    glEnd();

    // ----------------------------------------------------------------
    //  Goal boxes (small box inside each penalty area)
    // ----------------------------------------------------------------
    float gbD = fldW * 0.09f;  // depth of goal box
    float gbH = zoneH * 0.30f; // height of goal box
    float gbY1 = zoneCY - gbH * 0.5f;
    float gbY2 = zoneCY + gbH * 0.5f;

    // Left goal box
    glBegin(GL_LINE_LOOP);
    glVertex2f(fldX1 + fm, gbY1);
    glVertex2f(fldX1 + fm + gbD, gbY1);
    glVertex2f(fldX1 + fm + gbD, gbY2);
    glVertex2f(fldX1 + fm, gbY2);
    glEnd();

    // Right goal box
    glBegin(GL_LINE_LOOP);
    glVertex2f(fldX2 - fm, gbY1);
    glVertex2f(fldX2 - fm - gbD, gbY1);
    glVertex2f(fldX2 - fm - gbD, gbY2);
    glVertex2f(fldX2 - fm, gbY2);
    glEnd();

    // ----------------------------------------------------------------
    //  Penalty spots and D-arcs
    // ----------------------------------------------------------------
    float penSpotD = fldW * 0.14f; // distance from goal line to penalty spot
    float arcR = zoneH * 0.14f;

    // Left penalty spot
    float penSpotLX = fldX1 + fm + penSpotD;
    diskFan(penSpotLX, zoneCY, 0.005f / asp, 0.005f, 10, 240, 245, 240);
    // Left D-arc (right-facing, outside penalty box)
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 48; i++)
    {
        float a = -PI * 0.5f + (float)i / 48.f * PI;
        float px = penSpotLX + cosf(a) * arcR / asp;
        float py = zoneCY + sinf(a) * arcR;
        if (px > fldX1 + fm + penD) // only draw outside penalty box
            glVertex2f(px, py);
    }
    glEnd();

    // Right penalty spot
    float penSpotRX = fldX2 - fm - penSpotD;
    diskFan(penSpotRX, zoneCY, 0.005f / asp, 0.005f, 10, 240, 245, 240);
    // Right D-arc (left-facing, outside penalty box)
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 48; i++)
    {
        float a = PI * 0.5f + (float)i / 48.f * PI;
        float px = penSpotRX + cosf(a) * arcR / asp;
        float py = zoneCY + sinf(a) * arcR;
        if (px < fldX2 - fm - penD)
            glVertex2f(px, py);
    }
    glEnd();

    // ----------------------------------------------------------------
    //  Corner arcs — all four corners
    // ----------------------------------------------------------------
    float cArcR = zoneH * 0.06f;
    glLineWidth(1.4f);

    // Bottom-left
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 12; i++)
    {
        float a = (float)i / 12.f * (PI * 0.5f);
        glVertex2f(fldX1 + fm + cosf(a) * cArcR / asp,
                   fieldY1 + fm + sinf(a) * cArcR);
    }
    glEnd();
    // Bottom-right
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 12; i++)
    {
        float a = PI * 0.5f + (float)i / 12.f * (PI * 0.5f);
        glVertex2f(fldX2 - fm + cosf(a) * cArcR / asp,
                   fieldY1 + fm + sinf(a) * cArcR);
    }
    glEnd();
    // Top-left
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 12; i++)
    {
        float a = PI * 1.5f + (float)i / 12.f * (PI * 0.5f);
        glVertex2f(fldX1 + fm + cosf(a) * cArcR / asp,
                   fieldY2 - fm + sinf(a) * cArcR);
    }
    glEnd();
    // Top-right
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 12; i++)
    {
        float a = PI + (float)i / 12.f * (PI * 0.5f);
        glVertex2f(fldX2 - fm + cosf(a) * cArcR / asp,
                   fieldY2 - fm + sinf(a) * cArcR);
    }
    glEnd();

    // ----------------------------------------------------------------
    //  Goals — left and right side nets
    // ----------------------------------------------------------------
    float goalH = zoneH * 0.22f;    // goal opening height
    float goalDepth = fldW * 0.04f; // goal depth (into field)
    float goalY1 = zoneCY - goalH * 0.5f;
    float goalY2 = zoneCY + goalH * 0.5f;

    // Left goal net fill + frame
    solidQ(fldX1, goalY1, fldX1 + goalDepth, goalY2, 255, 255, 255);
    alphaQ(fldX1, goalY1, fldX1 + goalDepth, goalY2, 255, 255, 255, 55);
    glColor3ub(220, 220, 220);
    glLineWidth(0.6f);
    for (int g = 1; g <= 4; g++)
    {
        float gy = goalY1 + g * goalH / 5.f;
        glBegin(GL_LINES);
        glVertex2f(fldX1, gy);
        glVertex2f(fldX1 + goalDepth, gy);
        glEnd();
    }
    glBegin(GL_LINES);
    glVertex2f(fldX1 + goalDepth * 0.5f, goalY1);
    glVertex2f(fldX1 + goalDepth * 0.5f, goalY2);
    glEnd();

    // Right goal net fill + frame
    solidQ(fldX2 - goalDepth, goalY1, fldX2, goalY2, 255, 255, 255);
    alphaQ(fldX2 - goalDepth, goalY1, fldX2, goalY2, 255, 255, 255, 55);
    glColor3ub(220, 220, 220);
    glLineWidth(0.6f);
    for (int g = 1; g <= 4; g++)
    {
        float gy = goalY1 + g * goalH / 5.f;
        glBegin(GL_LINES);
        glVertex2f(fldX2 - goalDepth, gy);
        glVertex2f(fldX2, gy);
        glEnd();
    }
    glBegin(GL_LINES);
    glVertex2f(fldX2 - goalDepth * 0.5f, goalY1);
    glVertex2f(fldX2 - goalDepth * 0.5f, goalY2);
    glEnd();

    // ----------------------------------------------------------------
    //  Football field outer border
    // ----------------------------------------------------------------
    glColor3ub(28, 88, 24);
    glLineWidth(209.2f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(fldX1, fieldY1);
    glVertex2f(fldX2, fieldY1);
    glVertex2f(fldX2, fieldY2);
    glVertex2f(fldX1, fieldY2);
    glEnd();
}

// ================================================================
//  BRICK FOOTPATH (between road kerb and sports fields) //[Prottoy]
// ================================================================
static void drawBrickFootpath()
{
    float pathY1 = -0.725f; // top of kerb (roadY2 + 0.012)
    float pathY2 = -0.708f; // bottom of football field (fieldY1)
    float pathX1 = -1.0f, pathX2 = 1.0f;

    solidQ(pathX1, pathY1, pathX2, pathY2, 180, 120, 90);
    glColor3ub(140, 80, 60);
    glLineWidth(1.0f);
    float brickH = 0.020f;
    int nRows = (int)((pathY2 - pathY1) / brickH) + 1;
    for (int r = 0; r < nRows; r++)
    {
        float rowY = pathY1 + r * brickH;
        if (rowY > pathY2)
            break;
        glBegin(GL_LINES);
        glVertex2f(pathX1, rowY);
        glVertex2f(pathX2, rowY);
        glEnd();
        float stagger = (r % 2 == 0) ? 0.0f : 0.045f;
        for (float x = pathX1 + stagger; x < pathX2; x += 0.09f)
        {
            glBegin(GL_LINES);
            glVertex2f(x, rowY);
            glVertex2f(x, rowY + brickH);
            glEnd();
        }
    }
    glColor3ub(130, 70, 50);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(pathX1, pathY1);
    glVertex2f(pathX2, pathY1);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(pathX1, pathY2);
    glVertex2f(pathX2, pathY2);
    glEnd();
}

// ================================================================
//  WHITE APPLE BLOSSOM TREE  (replaces palm tree) // [Prottoy]
// ================================================================
static void drawWhiteBlossomTree(float x, float y, float phaseOffset)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(windSway * 0.50f * sinf(phaseOffset), 0.0f, 0.0f, 1.0f);
    glTranslatef(-x, -y, 0.0f);

    const float trunkH = 0.095f;

    // ── Trunk: 3-segment tapered with shadow/light split ──
    float tw[] = {0.010f, 0.007f, 0.005f, 0.004f};
    float ty2[] = {y, y + trunkH * 0.38f, y + trunkH * 0.68f, y + trunkH};
    float txb[] = {x, x - 0.002f, x + 0.002f, x + 0.003f};

    for (int s = 0; s < 3; s++)
    {
        // Shadow side (left)
        glColor3ub(52 + s * 4, 32 + s * 3, 10 + s * 2);
        glBegin(GL_QUADS);
        glVertex2f(txb[s] - tw[s], ty2[s]);
        glVertex2f(txb[s], ty2[s]);
        glVertex2f(txb[s + 1], ty2[s + 1]);
        glVertex2f(txb[s + 1] - tw[s + 1], ty2[s + 1]);
        glEnd();
        // Light side (right)
        glColor3ub(92 + s * 5, 58 + s * 4, 20 + s * 3);
        glBegin(GL_QUADS);
        glVertex2f(txb[s], ty2[s]);
        glVertex2f(txb[s] + tw[s], ty2[s]);
        glVertex2f(txb[s + 1] + tw[s + 1], ty2[s + 1]);
        glVertex2f(txb[s + 1], ty2[s + 1]);
        glEnd();
    }

    // Bark fissure lines
    glColor3ub(42, 26, 8);
    glLineWidth(0.55f);
    for (int b = 0; b < 7; b++)
    {
        float by2 = y + 0.006f + b * 0.012f;
        float bxc = txb[0] + (txb[3] - txb[0]) * (by2 - y) / trunkH;
        glBegin(GL_LINE_STRIP);
        glVertex2f(bxc - tw[1] + 0.001f, by2);
        glVertex2f(bxc + 0.001f, by2 + 0.004f);
        glVertex2f(bxc + tw[1] - 0.001f, by2 + 0.001f);
        glEnd();
    }

    // Exposed roots
    glColor3ub(62, 40, 12);
    glLineWidth(1.1f);
    float rootA[] = {-0.5f, 0.0f, 0.5f};
    for (int r = 0; r < 3; r++)
    {
        glBegin(GL_LINE_STRIP);
        glVertex2f(x + sinf(rootA[r]) * 0.005f, y + 0.003f);
        glVertex2f(x + sinf(rootA[r]) * 0.018f, y - 0.004f);
        glVertex2f(x + sinf(rootA[r]) * 0.026f, y - 0.001f);
        glEnd();
    }

    float topX = txb[3], topY = ty2[3];

    // ── 6 branches — wider spread, more natural angles ──
    struct Branch
    {
        float ex, ey;
        float lw;
    };
    Branch branches[] = {
        {topX - 0.055f, topY + 0.038f, 2.0f},
        {topX - 0.030f, topY + 0.058f, 1.6f},
        {topX - 0.008f, topY + 0.068f, 1.7f},
        {topX + 0.022f, topY + 0.064f, 1.6f},
        {topX + 0.048f, topY + 0.052f, 1.5f},
        {topX + 0.064f, topY + 0.030f, 1.4f},
    };
    for (int b2 = 0; b2 < 6; b2++)
    {
        glColor3ub(88, 58, 20);
        glLineWidth(branches[b2].lw);
        glBegin(GL_LINE_STRIP);
        glVertex2f(topX, topY);
        float mx = (topX + branches[b2].ex) * 0.5f + 0.005f * (b2 % 2 ? 1 : -1);
        float my = (topY + branches[b2].ey) * 0.5f + 0.010f;
        glVertex2f(mx, my);
        glVertex2f(branches[b2].ex, branches[b2].ey);
        glEnd();
        // Sub-twigs at each branch tip
        glColor3ub(72, 46, 16);
        glLineWidth(0.7f);
        glBegin(GL_LINES);
        glVertex2f(branches[b2].ex, branches[b2].ey);
        glVertex2f(branches[b2].ex - 0.012f, branches[b2].ey + 0.016f);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(branches[b2].ex, branches[b2].ey);
        glVertex2f(branches[b2].ex + 0.014f, branches[b2].ey + 0.012f);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(branches[b2].ex, branches[b2].ey);
        glVertex2f(branches[b2].ex + 0.004f, branches[b2].ey + 0.018f);
        glEnd();
    }

    // ══════════════════════════════════════════════
    //  CANOPY — 7 depth layers, wider & fuller dome
    // ══════════════════════════════════════════════

    // Layer 0 — deep cool shadow base (widens the silhouette)
    diskFan(x, topY + 0.005f, 0.068f, 0.058f, 24, 168, 160, 178);
    diskFan(x - 0.040f, topY - 0.002f, 0.050f, 0.044f, 22, 160, 152, 170);
    diskFan(x + 0.038f, topY - 0.004f, 0.048f, 0.042f, 22, 164, 156, 174);

    // Layer 1 — warm grey-white base fill
    diskFan(x, topY + 0.018f, 0.060f, 0.052f, 24, 208, 204, 216);
    diskFan(x - 0.028f, topY + 0.010f, 0.048f, 0.042f, 22, 202, 198, 210);
    diskFan(x + 0.026f, topY + 0.008f, 0.046f, 0.040f, 22, 205, 201, 213);

    // Layer 2 — cream mid fill
    diskFan(x - 0.018f, topY + 0.034f, 0.050f, 0.044f, 22, 228, 225, 235);
    diskFan(x + 0.020f, topY + 0.030f, 0.048f, 0.042f, 22, 225, 222, 232);
    diskFan(x, topY + 0.042f, 0.052f, 0.046f, 24, 232, 229, 238);
    diskFan(x - 0.042f, topY + 0.022f, 0.036f, 0.032f, 20, 220, 217, 228);
    diskFan(x + 0.040f, topY + 0.018f, 0.038f, 0.034f, 20, 222, 219, 230);

    // Layer 3 — bright white mid-canopy
    diskFan(x - 0.014f, topY + 0.054f, 0.042f, 0.036f, 22, 242, 240, 248);
    diskFan(x + 0.018f, topY + 0.050f, 0.040f, 0.034f, 22, 240, 238, 246);
    diskFan(x - 0.030f, topY + 0.044f, 0.034f, 0.030f, 20, 236, 234, 243); // For drawing ellipse, you give two radius one for, horizontal, and other for vertical
    diskFan(x + 0.034f, topY + 0.038f, 0.036f, 0.032f, 20, 238, 236, 245);

    // Layer 4 — luminous upper white
    diskFan(x + 0.005f, topY + 0.066f, 0.034f, 0.029f, 20, 250, 249, 254);
    diskFan(x - 0.022f, topY + 0.060f, 0.028f, 0.025f, 18, 248, 247, 253);
    diskFan(x + 0.026f, topY + 0.056f, 0.030f, 0.027f, 18, 249, 248, 254);
    diskFan(x - 0.040f, topY + 0.048f, 0.024f, 0.022f, 16, 244, 243, 250);
    diskFan(x + 0.042f, topY + 0.042f, 0.026f, 0.023f, 16, 246, 245, 251);

    // Layer 5 — pure white glowing crown
    glColor4ub(255, 254, 255, 230);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + 0.004f, topY + 0.080f);
    for (int k = 0; k <= 16; k++)
    {
        float a = k * 2.f * PI / 16.f;
        glVertex2f(x + 0.004f + cosf(a) * 0.022f, topY + 0.080f + sinf(a) * 0.019f);
    }
    glEnd();
    glColor4ub(252, 253, 255, 180);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x - 0.014f, topY + 0.073f);
    for (int k = 0; k <= 12; k++)
    {
        float a = k * 2.f * PI / 12.f;
        glVertex2f(x - 0.014f + cosf(a) * 0.016f, topY + 0.073f + sinf(a) * 0.014f);
    }
    glEnd();
    glColor4ub(255, 252, 255, 150);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + 0.020f, topY + 0.070f);
    for (int k = 0; k <= 10; k++)
    {
        float a = k * 2.f * PI / 10.f;
        glVertex2f(x + 0.020f + cosf(a) * 0.013f, topY + 0.070f + sinf(a) * 0.011f);
    }
    glEnd();

    // Layer 6 — soft pink blush tint patches (gives depth contrast)
    glColor4ub(255, 220, 235, 55);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x - 0.010f, topY + 0.028f);
    for (int k = 0; k <= 14; k++)
    {
        float a = k * 2.f * PI / 14.f;
        glVertex2f(x - 0.010f + cosf(a) * 0.038f, topY + 0.028f + sinf(a) * 0.030f);
    }
    glEnd();
    glColor4ub(240, 210, 228, 45);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + 0.028f, topY + 0.020f);
    for (int k = 0; k <= 12; k++)
    {
        float a = k * 2.f * PI / 12.f;
        glVertex2f(x + 0.028f + cosf(a) * 0.030f, topY + 0.020f + sinf(a) * 0.025f);
    }
    glEnd();

    // Scalloped silhouette outline
    glColor4ub(188, 182, 200, 140);
    glLineWidth(0.65f);
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k <= 36; k++)
    {
        float a = k * 2.f * PI / 36.f;
        float rr = 0.062f + 0.010f * sinf(a * 6.f) + 0.006f * cosf(a * 9.f);
        glVertex2f(x + cosf(a) * rr, topY + 0.044f + sinf(a) * rr * 0.82f);
    }
    glEnd();

    // ══════════════════════════════════════════════
    //  FLOWERS — 14 clusters, denser than before
    // ══════════════════════════════════════════════
    struct Flower
    {
        float ox, oy;
        float sz;
        float ang;
    };
    Flower flowers[] = {
        {-0.048f, 0.010f, 1.05f, 15.f},
        {-0.032f, 0.028f, 0.95f, 52.f},
        {-0.014f, 0.044f, 1.00f, 88.f},
        {0.006f, 0.054f, 1.05f, 124.f},
        {0.026f, 0.046f, 0.95f, 160.f},
        {0.044f, 0.032f, 0.90f, 196.f},
        {0.056f, 0.012f, 0.88f, 232.f},
        {-0.040f, 0.042f, 0.85f, 268.f},
        {-0.022f, 0.060f, 0.90f, 304.f},
        {0.014f, 0.065f, 0.88f, 340.f},
        {0.038f, 0.058f, 0.85f, 28.f},
        {-0.058f, 0.024f, 0.80f, 64.f},
        {0.002f, 0.032f, 0.92f, 100.f},
        {-0.028f, 0.016f, 0.88f, 148.f},
    };
    int nFlowers = 14;
    for (int fi = 0; fi < nFlowers; fi++)
    {
        float fx = x + flowers[fi].ox;
        float fy = topY + flowers[fi].oy;
        float fsz = flowers[fi].sz * 0.0072f;

        for (int p = 0; p < 5; p++)
        {
            float pa = flowers[fi].ang * PI / 180.f + p * 2.f * PI / 5.f;
            float pcx = fx + cosf(pa) * fsz * 0.80f;
            float pcy = fy + sinf(pa) * fsz * 0.80f;

            // Petal — white with faint warm blush at base
            glColor4ub(253, 250, 255, 245);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(pcx, pcy);
            for (int k = 0; k <= 14; k++)
            {
                float a = k * 2.f * PI / 14.f;
                glVertex2f(pcx + cosf(a) * fsz, pcy + sinf(a) * fsz * 0.85f);
            }
            glEnd();
            // Blush at petal base
            glColor4ub(255, 200, 220, 60);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(pcx, pcy);
            for (int k = 0; k <= 8; k++)
            {
                float a = k * 2.f * PI / 8.f;
                glVertex2f(pcx + cosf(a) * fsz * 0.42f, pcy + sinf(a) * fsz * 0.42f);
            }
            glEnd();
            // Petal vein
            glColor4ub(200, 180, 215, 80);
            glLineWidth(0.35f);
            glBegin(GL_LINES);
            glVertex2f(pcx, pcy);
            glVertex2f(pcx + cosf(pa) * fsz * 0.88f, pcy + sinf(pa) * fsz * 0.88f);
            glEnd();
        }
        // Golden stamen
        glColor3ub(255, 222, 50);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(fx, fy);
        for (int k = 0; k <= 10; k++) // center yellow circle
        {
            float a = k * 2.f * PI / 10.f;
            glVertex2f(fx + cosf(a) * fsz * 0.30f, fy + sinf(a) * fsz * 0.30f);
        }
        glEnd();
        // Stamen dot detail
        glColor3ub(200, 160, 20);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(fx, fy);
        for (int k = 0; k <= 8; k++)
        {
            float a = k * 2.f * PI / 8.f;
            glVertex2f(fx + cosf(a) * fsz * 0.14f, fy + sinf(a) * fsz * 0.14f);
        }
        glEnd();
    }

    // ── Static fallen petals at base ──
    float pdx[] = {x - 0.020f, x + 0.026f, x - 0.006f, x + 0.040f, x - 0.034f, x + 0.014f, x - 0.048f, x + 0.032f};
    float pdy[] = {y + 0.003f, y + 0.002f, y + 0.004f, y + 0.005f, y + 0.004f, y + 0.003f, y + 0.002f, y + 0.004f};
    float pda[] = {15.f, 48.f, 82.f, 130.f, 200.f, 310.f, 255.f, 175.f};
    for (int p = 0; p < 8; p++)
    {
        glPushMatrix();
        glTranslatef(pdx[p], pdy[p], 0.f);
        glRotatef(pda[p], 0.f, 0.f, 1.f);
        glColor4ub(253, 251, 255, 150);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.f, 0.f);
        for (int k = 0; k <= 8; k++)
        {
            float a = k * 2.f * PI / 8.f;
            glVertex2f(cosf(a) * 0.006f, sinf(a) * 0.004f);
        }
        glEnd();
        glPopMatrix();
    }

    // Summer: add jackfruits on white blossom trees
    if (summerMode)
    {
        float jackOffsets[][2] = {
            {-0.040f, 0.020f},
            {0.035f, 0.012f},
            {-0.010f, 0.045f},
            {0.020f, 0.030f}};
        int nJack = 4;
        for (int j = 0; j < nJack; j++)
        {
            float jx = x + jackOffsets[j][0];
            float jy = topY + jackOffsets[j][1];
            glColor3ub(110, 130, 40);
            diskFan(jx, jy, 0.009f, 0.010f, 12, 110, 130, 40);
            glColor3ub(90, 110, 28);
            diskFan(jx - 0.002f, jy + 0.002f, 0.007f, 0.008f, 10, 90, 110, 28);
            glColor3ub(70, 85, 22);
            for (int b = 0; b < 12; b++)
            {
                float bx = jx + (b % 3 - 1) * 0.004f;
                float by = jy + (b / 3 - 1) * 0.005f;
                diskFan(bx, by, 0.0018f, 0.0018f, 6, 70, 85, 22);
            }
            glColor3ub(72, 52, 18);
            glLineWidth(1.2f);
            glBegin(GL_LINES);
            glVertex2f(jx, jy + 0.013f);
            glVertex2f(jx + 0.003f, jy + 0.018f);
            glEnd();
        }
    }

    glPopMatrix();
}
static void drawOakTree(float x, float y, float phaseOffset) //[Prottoy]
{
    const float trunkH = 0.112f;

    // ── Wind sway pivot at trunk base ──
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(windSway * 0.75f * sinf(phaseOffset * 0.9f), 0.0f, 0.0f, 1.0f);
    glTranslatef(-x, -y, 0.0f);

    // ── Trunk: 3-segment tapered with shadow/light split ──
    float tw[] = {0.013f, 0.010f, 0.008f, 0.006f};
    float ty[] = {y, y + trunkH * 0.38f, y + trunkH * 0.68f, y + trunkH};
    float tx[] = {x, x - 0.002f, x + 0.001f, x + 0.003f};

    for (int s = 0; s < 3; s++)
    {
        glColor3ub(58 + s * 4, 38 + s * 3, 12 + s * 2);
        glBegin(GL_QUADS);
        glVertex2f(tx[s] - tw[s], ty[s]);
        glVertex2f(tx[s], ty[s]);
        glVertex2f(tx[s + 1], ty[s + 1]);
        glVertex2f(tx[s + 1] - tw[s + 1], ty[s + 1]);
        glEnd();
        glColor3ub(98 + s * 4, 65 + s * 3, 22 + s * 2); // Oaktree mai trunck was created using quads.
        glBegin(GL_QUADS);
        glVertex2f(tx[s], ty[s]);
        glVertex2f(tx[s] + tw[s], ty[s]);
        glVertex2f(tx[s + 1] + tw[s + 1], ty[s + 1]);
        glVertex2f(tx[s + 1], ty[s + 1]);
        glEnd();
    }

    // Bark fissure lines
    glColor3ub(48, 30, 8);
    glLineWidth(0.6f);
    for (int b = 0; b < 5; b++)
    {
        float by2 = y + 0.010f + b * 0.018f;
        float bxL = tx[0] + (tx[3] - tx[0]) * (by2 - y) / trunkH;
        glBegin(GL_LINE_STRIP);
        glVertex2f(bxL - tw[1] + 0.001f, by2);
        glVertex2f(bxL + 0.002f, by2 + 0.005f);
        glVertex2f(bxL + tw[1] - 0.001f, by2 + 0.002f);
        glEnd();
    }

    // Exposed roots
    glColor3ub(68, 44, 14);
    glLineWidth(1.2f);
    float rootAngles[] = {-0.4f, 0.f, 0.4f};
    for (int r = 0; r < 3; r++)
    {
        float ra = rootAngles[r];
        glBegin(GL_LINE_STRIP);
        glVertex2f(x + sinf(ra) * 0.006f, y + 0.004f);
        glVertex2f(x + sinf(ra) * 0.020f, y - 0.004f);
        glVertex2f(x + sinf(ra) * 0.028f, y - 0.002f);
        glEnd();
    }

    float foliageY = ty[3];

    // ── Foliage: 5 depth layers ──
    // Layer 1 — dark shadow base
    diskFan(x, foliageY + 0.008f, 0.064f, 0.056f, 22, 22, 72, 14);
    diskFan(x - 0.038f, foliageY + 0.000f, 0.046f, 0.040f, 20, 18, 65, 12);
    diskFan(x + 0.034f, foliageY + 0.004f, 0.050f, 0.044f, 20, 24, 78, 16);

    // Layer 2 — mid green
    diskFan(x - 0.018f, foliageY + 0.022f, 0.048f, 0.042f, 22, 34, 100, 20);
    diskFan(x + 0.022f, foliageY + 0.018f, 0.046f, 0.040f, 22, 30, 92, 18);
    diskFan(x, foliageY + 0.032f, 0.052f, 0.046f, 22, 36, 108, 22);

    // Layer 3 — brighter mid-canopy
    diskFan(x - 0.010f, foliageY + 0.046f, 0.040f, 0.035f, 20, 48, 124, 28);
    diskFan(x + 0.014f, foliageY + 0.042f, 0.038f, 0.033f, 20, 44, 118, 26);
    diskFan(x - 0.028f, foliageY + 0.036f, 0.032f, 0.028f, 18, 42, 114, 24);
    diskFan(x + 0.032f, foliageY + 0.030f, 0.034f, 0.030f, 18, 46, 120, 27);

    // Layer 4 — light upper crown
    diskFan(x + 0.006f, foliageY + 0.060f, 0.032f, 0.028f, 18, 60, 142, 36);
    diskFan(x - 0.020f, foliageY + 0.054f, 0.026f, 0.023f, 16, 56, 136, 33);
    diskFan(x + 0.024f, foliageY + 0.050f, 0.028f, 0.025f, 16, 58, 138, 34);

    // Layer 5 — sun-caught highlights
    glColor4ub(88, 178, 55, 100);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x - 0.006f, foliageY + 0.072f);
    for (int k = 0; k <= 14; k++)
    {
        float a = k * 2.f * PI / 14.f;
        glVertex2f(x - 0.006f + cosf(a) * 0.018f, foliageY + 0.072f + sinf(a) * 0.016f);
    }
    glEnd();
    glColor4ub(102, 195, 65, 75);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + 0.012f, foliageY + 0.066f);
    for (int k = 0; k <= 12; k++)
    {
        float a = k * 2.f * PI / 12.f;
        glVertex2f(x + 0.012f + cosf(a) * 0.014f, foliageY + 0.066f + sinf(a) * 0.012f);
    }
    glEnd();

    // ── Scalloped outline silhouette ──
    glColor4ub(40, 112, 24, 160);
    glLineWidth(0.7f);
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k <= 28; k++)
    {
        float a = k * 2.f * PI / 28.f;
        float rr = 0.060f + 0.010f * sinf(a * 4.f) + 0.006f * cosf(a * 7.f);
        glVertex2f(x + cosf(a) * rr, foliageY + 0.040f + sinf(a) * rr * 0.82f); // scattered line outside the tree
    }
    glEnd();

    // ── Small twig details ──
    glColor3ub(72, 50, 18);
    glLineWidth(0.9f);
    float twigAng[] = {-1.1f, -0.5f, 0.3f, 0.9f};
    for (int t2 = 0; t2 < 4; t2++)
    {
        float ta = twigAng[t2];
        glBegin(GL_LINES);
        glVertex2f(tx[3], ty[3]);
        glVertex2f(tx[3] + cosf(ta) * 0.020f, ty[3] + sinf(ta) * 0.020f);
        glEnd();
    }

    // ── FRUITS (apples) — scattered inside lower canopy ──
    struct FruitDef
    {
        float ox, oy;
        unsigned char r, g, b;
    };
    FruitDef fruits[] = {
        // Red apples
        {-0.010f, 0.038f, 215, 42, 30},
        {-0.048f, 0.010f, 208, 40, 28},
        // Yellow-green apples
        {0.018f, 0.052f, 160, 195, 48},
        {-0.022f, 0.046f, 155, 188, 44},
    };
    int nFruits = 4;
    for (int f = 0; f < nFruits; f++)
    {
        float fx = x + fruits[f].ox;
        float fy = foliageY + fruits[f].oy;
        float fr = 0.0068f;

        // Short stem
        glColor3ub(72, 50, 18);
        glLineWidth(0.8f); // top branch of the fruit
        glBegin(GL_LINES);
        glVertex2f(fx, fy + fr);
        glVertex2f(fx + 0.001f, fy + fr + 0.006f);
        glEnd();

        // Apple body
        diskFan(fx, fy, fr, fr, 14, fruits[f].r, fruits[f].g, fruits[f].b); // apple body made with user defined function

        // Highlight
        glColor4ub(255, 255, 220, 110);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(fx - fr * 0.28f, fy + fr * 0.30f); //->A small, semi‑transparent ellipse placed near the top‑left of the apple.
        for (int k = 0; k <= 8; k++)
        {
            float a = k * 2.f * PI / 8.f;
            glVertex2f(fx - fr * 0.28f + cosf(a) * fr * 0.38f,
                       fy + fr * 0.30f + sinf(a) * fr * 0.32f);
        }
        glEnd();

        // Leaf at stem tip
        glColor3ub(38, 112, 28);
        glBegin(GL_TRIANGLES);
        glVertex2f(fx + 0.001f, fy + fr + 0.006f); //-> a small leaf effect at the top of the fruit
        glVertex2f(fx + 0.007f, fy + fr + 0.010f);
        glVertex2f(fx - 0.003f, fy + fr + 0.011f); // a leaf type effect
        glEnd();
    }

    // Summer: add mangoes on oak trees
    if (summerMode)
    {
        float mangoOffsets[][2] = {
            {-0.035f, 0.018f},
            {0.028f, 0.025f},
            {-0.008f, 0.045f},
            {0.045f, 0.010f},
            {-0.025f, 0.050f}};
        int nMangos = 5;
        for (int m = 0; m < nMangos; m++)
        {
            float mx = x + mangoOffsets[m][0];
            float my = foliageY + mangoOffsets[m][1];
            glColor3ub(240, 160, 40);
            diskFan(mx, my, 0.007f, 0.010f, 12, 240, 160, 40);
            glColor3ub(220, 130, 30);
            diskFan(mx - 0.001f, my + 0.002f, 0.006f, 0.008f, 10, 220, 130, 30);
            glColor3ub(100, 160, 40);
            diskFan(mx + 0.002f, my - 0.006f, 0.003f, 0.004f, 8, 100, 160, 40);
            glColor3ub(88, 64, 24);
            glLineWidth(0.9f);
            glBegin(GL_LINES);
            glVertex2f(mx, my + 0.010f);
            glVertex2f(mx - 0.002f, my + 0.014f);
            glEnd();
            glColor3ub(48, 128, 32);
            glBegin(GL_TRIANGLES);
            glVertex2f(mx - 0.002f, my + 0.014f);
            glVertex2f(mx - 0.009f, my + 0.017f);
            glVertex2f(mx - 0.004f, my + 0.011f);
            glEnd();
        }
    }

    glPopMatrix(); // end wind sway
}

static void drawBlossomTree(float x, float y, float phaseOffset)
{
    // Wind sway: rotate around base (blossom trees sway more gently)
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(windSway * 0.97f * sinf(phaseOffset * 1.3f), 0.0f, 0.0f, 1.0f);
    glTranslatef(-x, -y, 0.0f);
    const float trunkH = 0.090f;

    // ── Slender trunk with lean ──
    float tw[] = {0.008f, 0.006f, 0.005f, 0.004f};
    float ty2[] = {y, y + trunkH * 0.35f, y + trunkH * 0.68f, y + trunkH};
    float txb[] = {x, x + 0.003f, x + 0.005f, x + 0.004f};

    for (int s = 0; s < 3; s++)
    {
        glColor3ub(64 + s * 3, 40 + s * 2, 20 + s);
        glBegin(GL_QUADS);
        glVertex2f(txb[s] - tw[s], ty2[s]);
        glVertex2f(txb[s] + tw[s], ty2[s]);
        glVertex2f(txb[s + 1] + tw[s + 1], ty2[s + 1]);
        glVertex2f(txb[s + 1] - tw[s + 1], ty2[s + 1]);
        glEnd();
        // Light seam
        glColor4ub(112, 72, 35, 80);
        glBegin(GL_LINES);
        glVertex2f(txb[s] + tw[s] * 0.3f, ty2[s]);
        glVertex2f(txb[s + 1] + tw[s + 1] * 0.3f, ty2[s + 1]);
        glEnd();
    }
    // Fine horizontal bark lines
    glColor3ub(52, 33, 16);
    glLineWidth(0.5f);
    for (int b = 0; b < 6; b++)
    {
        float by2 = y + 0.008f + b * 0.013f;
        float bxc = txb[0] + (txb[3] - txb[0]) * (by2 - y) / trunkH;
        glBegin(GL_LINES);
        glVertex2f(bxc - 0.006f, by2);
        glVertex2f(bxc + 0.007f, by2 + 0.001f);
        glEnd();
    }

    float bBase = y + trunkH * 0.62f;
    float topX = txb[3], topY = ty2[3];

    // ── Visible branching structure: 5 branches ──
    struct Branch
    {
        float ex, ey;
        float lw;
    };
    Branch branches[] = {
        {topX - 0.048f, topY + 0.044f, 1.8f},
        {topX - 0.025f, topY + 0.060f, 1.5f},
        {topX + 0.005f, topY + 0.065f, 1.6f},
        {topX + 0.038f, topY + 0.056f, 1.5f},
        {topX + 0.058f, topY + 0.036f, 1.4f},
    };
    for (int b2 = 0; b2 < 5; b2++)
    {
        glColor3ub(82, 54, 28);
        glLineWidth(branches[b2].lw);
        glBegin(GL_LINE_STRIP);
        glVertex2f(topX, topY);
        float mx = (topX + branches[b2].ex) * 0.5f + 0.004f * (b2 % 2 ? 1 : -1);
        float my = (topY + branches[b2].ey) * 0.5f + 0.008f;
        glVertex2f(mx, my);
        glVertex2f(branches[b2].ex, branches[b2].ey);
        glEnd();
        // Sub-twigs
        glColor3ub(75, 48, 24);
        glLineWidth(0.7f);
        glBegin(GL_LINES);
        glVertex2f(branches[b2].ex, branches[b2].ey);
        glVertex2f(branches[b2].ex - 0.010f, branches[b2].ey + 0.014f);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(branches[b2].ex, branches[b2].ey);
        glVertex2f(branches[b2].ex + 0.012f, branches[b2].ey + 0.010f);
        glEnd();
    }

    // ── Blossom canopy: 6-layer depth model ──
    // Layer 1 — deep shadow interior
    if (summerMode)
    {
        diskFan(x, topY + 0.012f, 0.055f, 0.048f, 22, 180, 130, 50);
        diskFan(x - 0.032f, topY + 0.006f, 0.040f, 0.036f, 20, 170, 120, 45);
        diskFan(x + 0.030f, topY + 0.002f, 0.038f, 0.034f, 20, 175, 125, 48);
    }
    else
    {
        // Original spring/pink colors
        diskFan(x, topY + 0.012f, 0.055f, 0.048f, 22, 170, 108, 138);
        diskFan(x - 0.032f, topY + 0.006f, 0.040f, 0.036f, 20, 162, 100, 130);
        diskFan(x + 0.030f, topY + 0.002f, 0.038f, 0.034f, 20, 168, 106, 136);
    }

    // Layer 2 — base fill
    if (summerMode)
    {
        diskFan(x - 0.015f, topY + 0.030f, 0.042f, 0.036f, 22, 235, 180, 60);
        diskFan(x + 0.018f, topY + 0.026f, 0.040f, 0.034f, 22, 230, 175, 55);
        diskFan(x, topY + 0.038f, 0.044f, 0.038f, 22, 240, 185, 65);
    }
    else
    {
        // Original spring/pink colors
        diskFan(x - 0.015f, topY + 0.030f, 0.042f, 0.036f, 22, 208, 140, 168);
        diskFan(x + 0.018f, topY + 0.026f, 0.040f, 0.034f, 22, 204, 136, 164);
        diskFan(x, topY + 0.038f, 0.044f, 0.038f, 22, 212, 144, 172);
    }

    // Layer 3 — mid canopy
    if (summerMode)
    {
        diskFan(x - 0.012f, topY + 0.050f, 0.036f, 0.030f, 20, 250, 210, 70);
        diskFan(x + 0.016f, topY + 0.046f, 0.034f, 0.028f, 20, 248, 205, 68);
        diskFan(x - 0.026f, topY + 0.040f, 0.028f, 0.025f, 18, 245, 200, 65);
        diskFan(x + 0.030f, topY + 0.034f, 0.030f, 0.026f, 18, 252, 208, 72);
    }
    else
    {
        // Original spring/pink colors
        diskFan(x - 0.012f, topY + 0.050f, 0.036f, 0.030f, 20, 228, 158, 185);
        diskFan(x + 0.016f, topY + 0.046f, 0.034f, 0.028f, 20, 224, 154, 182);
        diskFan(x - 0.026f, topY + 0.040f, 0.028f, 0.025f, 18, 220, 150, 178);
        diskFan(x + 0.030f, topY + 0.034f, 0.030f, 0.026f, 18, 226, 156, 184);
    }

    // Layer 4 — bright upper
    if (summerMode)
    {
        diskFan(x + 0.004f, topY + 0.062f, 0.028f, 0.024f, 18, 255, 230, 110);
        diskFan(x - 0.018f, topY + 0.056f, 0.022f, 0.020f, 16, 255, 225, 100);
        diskFan(x + 0.022f, topY + 0.052f, 0.024f, 0.022f, 16, 255, 228, 105);
    }
    else
    {
        // Original spring/pink colors
        diskFan(x + 0.004f, topY + 0.062f, 0.028f, 0.024f, 18, 244, 178, 205);
        diskFan(x - 0.018f, topY + 0.056f, 0.022f, 0.020f, 16, 240, 172, 200);
        diskFan(x + 0.022f, topY + 0.052f, 0.024f, 0.022f, 16, 242, 175, 202);
    }

    // Layer 5 — light-caught petal tips (always same - white/pink highlights)
    glColor4ub(255, 215, 230, 200);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + 0.003f, topY + 0.074f);
    for (int k = 0; k <= 14; k++)
    {
        float a = k * 2.f * PI / 14.f;
        glVertex2f(x + 0.003f + cosf(a) * 0.016f, topY + 0.074f + sinf(a) * 0.014f);
    }
    glEnd();
    glColor4ub(255, 228, 240, 140);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x - 0.012f, topY + 0.068f);
    for (int k = 0; k <= 10; k++)
    {
        float a = k * 2.f * PI / 10.f;
        glVertex2f(x - 0.012f + cosf(a) * 0.011f, topY + 0.068f + sinf(a) * 0.010f);
    }
    glEnd();

    // Layer 6 — scalloped outline silhouette (always same)
    glColor4ub(180, 115, 148, 160);
    glLineWidth(0.6f);
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k <= 30; k++)
    {
        float a = k * 2.f * PI / 30.f;
        float rr = 0.054f + 0.008f * sinf(a * 5.f) + 0.005f * cosf(a * 8.f);
        glVertex2f(x + cosf(a) * rr, topY + 0.042f + sinf(a) * rr * 0.80f);
    }
    glEnd();

    // ── Falling petals near canopy base (static detail) ──
    float pdx[] = {x - 0.022f, x + 0.028f, x - 0.008f, x + 0.042f, x - 0.038f, x + 0.016f};
    float pdy[] = {topY - 0.006f, topY - 0.010f, topY - 0.004f, topY + 0.004f, topY + 0.002f, topY - 0.008f};
    float pda[] = {12.f, 45.f, 78.f, 125.f, 200.f, 310.f};
    for (int p = 0; p < 6; p++)
    {
        glPushMatrix();
        glTranslatef(pdx[p], pdy[p], 0.f);
        glRotatef(pda[p], 0.f, 0.f, 1.f);
        glColor4ub(245, 185, 210, 130);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.f, 0.f);
        for (int k = 0; k <= 8; k++)
        {
            float a = k * 2.f * PI / 8.f;
            glVertex2f(cosf(a) * 0.005f, sinf(a) * 0.004f);
        }
        glEnd();
        glPopMatrix();
    }

    // Summer: add mangoes on pink blossom trees
    if (summerMode)
    {
        float mangoOffsets[][2] = {
            {-0.030f, 0.022f},
            {0.025f, 0.018f},
            {-0.010f, 0.048f},
            {0.040f, 0.008f}};
        int nMangos = 4;
        for (int m = 0; m < nMangos; m++)
        {
            float mx = x + mangoOffsets[m][0];
            float my = topY + mangoOffsets[m][1];
            glColor3ub(245, 170, 50);
            diskFan(mx, my, 0.007f, 0.010f, 12, 245, 170, 50);
            glColor3ub(220, 140, 35);
            diskFan(mx - 0.001f, my + 0.002f, 0.006f, 0.008f, 10, 220, 140, 35);
            glColor3ub(100, 160, 40);
            diskFan(mx + 0.002f, my - 0.006f, 0.003f, 0.004f, 8, 100, 160, 40);
            glColor3ub(88, 64, 24);
            glLineWidth(0.9f);
            glBegin(GL_LINES);
            glVertex2f(mx, my + 0.010f);
            glVertex2f(mx - 0.002f, my + 0.014f);
            glEnd();
            glColor3ub(48, 128, 32);
            glBegin(GL_TRIANGLES);
            glVertex2f(mx - 0.002f, my + 0.014f);
            glVertex2f(mx - 0.009f, my + 0.017f);
            glVertex2f(mx - 0.004f, my + 0.011f);
            glEnd();
        }
    }

    glPopMatrix();
}

static void drawLampPost(float x, float y, float side) //[Prottoy]
{
    float postH = 0.195f;

    // --- Structural post (always visible, no change) ---
    solidQ(x - 0.016f, y, x + 0.016f, y + 0.010f, 82, 78, 68);
    solidQ(x - 0.012f, y + 0.008f, x + 0.012f, y + 0.018f, 72, 68, 58);
    solidQ(x - 0.006f, y + 0.016f, x + 0.006f, y + postH * 0.48f, 68, 64, 54);
    solidQ(x - 0.009f, y + postH * 0.46f, x + 0.009f, y + postH * 0.50f, 60, 57, 48);
    solidQ(x - 0.004f, y + postH * 0.48f, x + 0.004f, y + postH * 0.86f, 65, 62, 52);

    // Arm
    float armBaseY = y + postH * 0.86f;
    float armEndX = x + side * 0.046f;
    float armEndY = armBaseY + 0.028f;
    float armMidX = x + side * 0.022f;
    float armMidY = armBaseY + 0.018f;
    glColor3ub(62, 59, 50);
    glLineWidth(2.8f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x, armBaseY);
    glVertex2f(armMidX, armMidY);
    glVertex2f(armEndX, armEndY);
    glEnd();

    float lanX = armEndX, lanY = armEndY;

    // ===== NIGHT‑TIME LIGHTING (fades with daylight) =====
    float night = 1.0f - getDaylight(); // 0 = full day, 1 = night
    if (night > 0.02f)                  // only draw lights when it's noticeably dark
    {
        int coneSegs = 40;
        float coneR = 0.090f;
        float coneDropY = 0.090f;

        // Outer cone
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(255, 232, 140, (unsigned char)(200 * night)); // If the night=0 the outer cone vanishes
        glVertex2f(lanX, lanY - 0.006f);
        for (int i = 0; i <= coneSegs; i++)
        {
            float t = (float)i / coneSegs;
            float cx = lanX + (t - 0.5f) * 1.95f * coneR;
            float cy = lanY - 0.006f - coneDropY;
            glColor4ub(255, 218, 100, 0);
            glVertex2f(cx, cy);
        }
        glEnd();

        // Inner cone
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(255, 245, 180, (unsigned char)(160 * night)); // If the night=0 the outer cone vanishes
        glVertex2f(lanX, lanY - 0.006f);
        for (int i = 0; i <= coneSegs; i++)
        {
            float t = (float)i / coneSegs;
            float cx = lanX + (t - 0.5f) * 2.f * coneR * 0.45f;
            float cy = lanY - 0.006f - coneDropY * 0.85f;
            glColor4ub(255, 230, 130, 0);
            glVertex2f(cx, cy);
        }
        glEnd();

        // Ground pool (concentric rings)
        float poolCX = lanX;
        float poolCY = y + 0.004f;
        float poolR = 0.072f;
        int poolSeg = 32;

        // Outer ring
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(255, 218, 100, 0);
        glVertex2f(poolCX, poolCY);
        for (int i = 0; i <= poolSeg; i++)
        {
            float a = i * 2.f * PI / poolSeg;
            glVertex2f(poolCX + cosf(a) * poolR, poolCY + sinf(a) * poolR * 0.28f);
        }
        glEnd();

        // Mid ring
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(255, 220, 110, (unsigned char)(28 * night));
        glVertex2f(poolCX, poolCY);
        for (int i = 0; i <= poolSeg; i++)
        {
            float a = i * 2.f * PI / poolSeg;
            glColor4ub(255, 218, 100, 0);
            glVertex2f(poolCX + cosf(a) * poolR, poolCY + sinf(a) * poolR * 0.28f);
        }
        glEnd();

        // Inner bright core
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(255, 235, 150, (unsigned char)(55 * night));
        glVertex2f(poolCX, poolCY);
        for (int i = 0; i <= poolSeg; i++)
        {
            float a = i * 2.f * PI / poolSeg;
            glColor4ub(255, 220, 110, 0);
            glVertex2f(poolCX + cosf(a) * poolR * 0.45f, poolCY + sinf(a) * poolR * 0.45f * 0.28f); // The ground pool light effect
        }
        glEnd();
    }

    // --- Lantern housing (always visible) ---
    solidQ(lanX - 0.022f, lanY - 0.012f, lanX + 0.022f, lanY + 0.004f, 52, 50, 42);

    // Glass panel (glow fades with night)
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(255, 252, 200, (unsigned char)(255 * night));
    glVertex2f(lanX, lanY - 0.005f);
    float glassHW = 0.016f;
    glColor4ub(255, 190, 80, (unsigned char)(200 * night));
    glVertex2f(lanX - glassHW, lanY - 0.010f);
    glColor4ub(255, 200, 90, (unsigned char)(200 * night));
    glVertex2f(lanX + glassHW, lanY - 0.010f);
    glColor4ub(255, 195, 85, (unsigned char)(200 * night));
    glVertex2f(lanX + glassHW, lanY + 0.002f);
    glColor4ub(255, 188, 80, (unsigned char)(200 * night));
    glVertex2f(lanX - glassHW, lanY + 0.002f);
    glColor4ub(255, 190, 80, (unsigned char)(200 * night));
    glVertex2f(lanX - glassHW, lanY - 0.010f);
    glEnd();

    // Lantern cap
    solidQ(lanX - 0.024f, lanY + 0.002f, lanX + 0.024f, lanY + 0.008f, 45, 43, 36);

    // Filament hot‑spot (fades with night)
    if (night > 0.02f)
    {
        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(255, 255, 255, (unsigned char)(240 * night));
        glVertex2f(lanX, lanY - 0.004f);
        for (int i = 0; i <= 20; i++)
        {
            float a = i * 2.f * PI / 20;
            glColor4ub(255, 240, 160, 0);
            glVertex2f(lanX + cosf(a) * 0.010f, lanY - 0.004f + sinf(a) * 0.010f);
        }
        glEnd();
    }

    // Top finial (always visible)
    solidQ(x - 0.005f, y + postH * 0.855f, x + 0.005f, y + postH, 58, 55, 46);
    diskFan(x, y + postH, 0.006f, 0.006f, 10, 52, 50, 42);
}
static void drawRoadsideItems() //[Prottoy]
{
    float topKerbY = -0.708f;
    float botKerbY = -0.990f;

    // Phase offset counter – increments for each tree to give unique sway
    int phaseIdx = 1;

    // ----- TOP ROW -----
    struct
    {
        float x;
        int type;
        float side;
    } topItems[] = {
        {-0.92f, 1, +1},
        {-0.74f, 3, -1},
        {-0.56f, 2, +1},
        {-0.38f, 0, +1},
        {-0.18f, 3, +1},
        {-0.00f, 1, -1}, //+ or -1 is giving the direction of left or right
        {0.16f, 2, +1},
        {0.34f, 0, +1},
        {0.52f, 3, -1},
        {0.68f, 1, +1},
        {0.84f, 2, +1},
    };
    int nTop = sizeof(topItems) / sizeof(topItems[0]);
    for (int i = 0; i < nTop; i++)
    {
        float phase = (float)(phaseIdx * 43.0f);
        if (topItems[i].type == 3)
            drawLampPost(topItems[i].x, topKerbY, topItems[i].side);
        else if (topItems[i].type == 0)
        {
            drawWhiteBlossomTree(topItems[i].x, topKerbY, phase);
            phaseIdx++;
        } // Placing all the raodside item.
        else if (topItems[i].type == 1)
        {
            drawOakTree(topItems[i].x, topKerbY, phase);
            phaseIdx++;
        }
        else
        {
            drawBlossomTree(topItems[i].x, topKerbY, phase);
            phaseIdx++;
        }
    }

    // ----- BOTTOM ROW -----
    struct
    {
        float x;
        int type;
        float side;
    } botItems[] = {
        {-0.85f, 0, +1},
        {-0.67f, 3, +1},
        {-0.49f, 1, +1},
        {-0.31f, 2, +1},
        {-0.13f, 3, +1},
        {0.05f, 1, +1},
        {0.23f, 2, +1},
        {0.41f, 0, +1},
        {0.59f, 3, +1},
        {0.77f, 1, +1},
        {0.95f, 2, +1},
    };
    int nBot = sizeof(botItems) / sizeof(botItems[0]);
    for (int i = 0; i < nBot; i++)
    {
        float phase = (float)(phaseIdx * 43.0f); // determining the movement o the tree.
        if (botItems[i].type == 3)
            drawLampPost(botItems[i].x, botKerbY, botItems[i].side);
        else if (botItems[i].type == 0)
        {
            drawWhiteBlossomTree(botItems[i].x, botKerbY, phase);
            phaseIdx++;
        }
        else if (botItems[i].type == 1)
        {
            drawOakTree(botItems[i].x, botKerbY, phase);
            phaseIdx++;
        }
        else
        {
            drawBlossomTree(botItems[i].x, botKerbY, phase);
            phaseIdx++;
        }
    }
}
// ================================================================
//  DRAW A SINGLE WHITE APPLE BLOSSOM PETAL
// ================================================================
static void drawSakuraPetal(float cx, float cy, float sz, float angleDeg, float alpha, int colorType) // [Prottoy]
{
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(angleDeg, 0.0f, 0.0f, 1.0f);
    glScalef(sz, sz, 0.75f); // squeezes the y-axis of the paddle by 25%

    const int SEG = 14;
    const float lobR = (colorType == 0) ? 0.0085f : 0.0082f; // pink slightly larger
    const float dist = (colorType == 0) ? 0.0070f : 0.0065f;

    for (int lobe = 0; lobe < 5; lobe++)
    {
        float la = lobe * 2.f * PI / 5.f;
        float lcx = cosf(la) * dist;
        float lcy = sinf(la) * dist;

        unsigned char r, g, b;
        // In drawSakuraPetal(), replace the summer color section:
        if (colorType == 0)
        {
            // Pink sakura for spring, golden-yellow for summer
            if (summerMode)
            {
                r = 255;
                g = 200 + lobe * 8;
                b = 60;
            }
            else
            {
                r = 238;
                g = 156 + lobe * 4;
                b = 185;
            }
        }
        else
        {
            // White apple blossom (with faint pink blush)
            r = 252;
            g = 244 + lobe;
            b = 250;
        }

        glColor4ub(r, g, b, (unsigned char)(alpha * 255));
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(lcx, lcy);
        for (int s = 0; s <= SEG; s++)
        { // The paddles drew using circle and then made ellipes using scalling
            float sa = s * 2.f * PI / SEG;
            glVertex2f(lcx + cosf(sa) * lobR, lcy + sinf(sa) * lobR); // multiple ellipses are made to make a paddle.
        }
        glEnd();

        // Soft inner highlight
        if (colorType == 0)
        {
            glColor4ub(255, 210, 228, (unsigned char)(alpha * 140));
        }
        else
        {
            glColor4ub(255, 255, 255, (unsigned char)(alpha * 130));
        }
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(lcx + lobR * 0.18f, lcy + lobR * 0.22f);
        for (int s = 0; s <= 10; s++)
        {
            float sa = s * 2.f * PI / 10;
            glVertex2f(lcx + lobR * 0.18f + cosf(sa) * lobR * 0.40f,
                       lcy + lobR * 0.22f + sinf(sa) * lobR * 0.38f);
        }
        glEnd();
    }

    // Yellow stamen
    glColor4ub(255, 235, 120, (unsigned char)(alpha * 200));
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.f, 0.f);
    for (int s = 0; s <= 10; s++)
    {
        float sa = s * 2.f * PI / 10;
        glVertex2f(cosf(sa) * 0.0028f, sinf(sa) * 0.0028f);
    }
    glEnd();

    glPopMatrix();
}

// ================================================================
//  INIT PETALS — call once from your init()
// ================================================================
static void initPetals() // [Prottoy]
{
    for (int i = 0; i < MAX_FALLING; i++)
    {
        int bi = (int)(petalRand() * nBlossom);
        float bxSrc = blossomX[bi];

        bool isTopRow = (bi == 0 || bi == 1 || bi == 4 || bi == 5 || bi == 6);
        bool isWhite = (bi <= 3); // first 4 are white blossom trees
        float baseY = isTopRow ? -0.708f : -0.990f;

        float canopyBot, canopyTop;
        if (isWhite)
        {
            // New white blossom: trunkH=0.090, canopy starts at topY
            float topY = baseY + 0.090f; // trunk top
            canopyBot = topY + 0.002f;   // canopy base (matches Layer 1 start)
            canopyTop = topY + 0.082f;   // canopy top  (matches Layer 5 crown tip)
        }
        else
        {
            float topY = baseY + 0.090f; // pink blossom trunk top
            canopyBot = topY + 0.002f;
            canopyTop = topY + 0.082f;
        }

        // initislizing each paddles different attributes
        fallingPetals[i].x = bxSrc + (petalRand() - 0.5f) * 0.14f;
        fallingPetals[i].y = canopyBot + petalRand() * (canopyTop - canopyBot);
        fallingPetals[i].vx = (petalRand() - 0.5f) * 0.0025f;
        fallingPetals[i].vy = -(0.0014f + petalRand() * 0.0010f);
        fallingPetals[i].angle = petalRand() * 360.f;
        fallingPetals[i].angSpeed = (petalRand() - 0.5f) * 2.4f;
        fallingPetals[i].size = 0.15f + petalRand() * 0.27f;
        fallingPetals[i].swing = petalRand() * 2.f * PI;
        fallingPetals[i].swingSpeed = 0.024f + petalRand() * 0.036f;
        fallingPetals[i].swingAmp = 0.0012f + petalRand() * 0.0018f;
        fallingPetals[i].active = true;
        fallingPetals[i].colorType = isWhite ? 1 : 0; // 1 = white, 0 = pink
    }

    // initalizing the resting paddles
    restingCount = MAX_RESTING;
    for (int i = 0; i < MAX_RESTING; i++)
    {
        int bi = (int)(petalRand() * nBlossom);
        float bxSrc = blossomX[bi];
        bool isWhite = (bi <= 3);
        restingPetals[i].x = bxSrc + (petalRand() - 0.5f) * 0.60f;
        float zones[] = {
            -0.712f, -0.715f, -0.740f, -0.780f,
            -0.820f, -0.860f, -0.900f, -0.940f, -0.970f};
        restingPetals[i].y = zones[(int)(petalRand() * 9)] + petalRand() * 0.010f;
        restingPetals[i].angle = petalRand() * 360.f;
        restingPetals[i].size = 0.20f + petalRand() * 0.35f;
        restingPetals[i].alpha = 0.45f + petalRand() * 0.38f;
        restingPetals[i].colorType = isWhite ? 1 : 0;
    }
}

// ================================================================
//  UPDATE PETAL PHYSICS — call from glutTimerFunc callback
// ================================================================
static void updatePetals() // [Prottoy]
{
    // Wind sway update — each call advances the oscillation
    for (int i = 0; i < MAX_FALLING; i++) // The actual implementation is done here.
    {
        if (!fallingPetals[i].active)
            continue;
        Petal &p = fallingPetals[i];
        p.swing += p.swingSpeed;                  // changing the swing speed
        p.x += p.vx + cosf(p.swing) * p.swingAmp; // changing the movement of horizontal axix
        p.y += p.vy;                              // the movement of y axis stayed constant
        p.angle += p.angSpeed;                    // changing the movement of the angle at costant value.

        if (p.y < -0.995f)
        {
            int slot = restingCount % MAX_RESTING;
            restingPetals[slot].x = p.x;
            float r = petalRand();
            if (r < 0.20f)
                restingPetals[slot].y = -0.710f + petalRand() * 0.012f; // Placin where it will rest on the footpath area or on the road
            else
                restingPetals[slot].y = -0.730f - petalRand() * 0.260f;
            restingPetals[slot].angle = petalRand() * 360.f; // these are the propertise of resting paddles
            restingPetals[slot].size = p.size * 0.80f;
            restingPetals[slot].alpha = 0.42f + petalRand() * 0.35f;
            restingPetals[slot].colorType = p.colorType; // preserve colour when landing
            restingCount++;
            if (restingCount > MAX_RESTING * 20)
                restingCount = MAX_RESTING;

            // Respawn from a random tree
            int bi = (int)(petalRand() * nBlossom); // choose a random tree to start on.
            float bxNew = blossomX[bi];
            bool isTopRow = (bi == 0 || bi == 1 || bi == 4 || bi == 5 || bi == 6); // start respwaning from a arandom tree.
            bool isWhite = (bi <= 3);
            float baseY = isTopRow ? -0.708f : -0.990f;

            float canopyBot, canopyTop;
            if (isWhite)
            {
                // New white blossom: trunkH=0.090, canopy starts at topY
                float topY = baseY + 0.090f; // trunk top
                canopyBot = topY + 0.002f;   // canopy base (matches Layer 1 start)
                canopyTop = topY + 0.082f;   // canopy top  (matches Layer 5 crown tip)
            }
            else
            {
                float topY = baseY + 0.090f;
                canopyBot = topY + 0.002f;
                canopyTop = topY + 0.082f;
            }

            // setting attributes for every respawning paddles
            p.x = bxNew + (petalRand() - 0.5f) * 0.14f;
            p.y = canopyBot + petalRand() * (canopyTop - canopyBot); // repositioning the paddles
            p.vx = (petalRand() - 0.5f) * 0.0025f;
            p.vy = -(0.0014f + petalRand() * 0.0013f);
            p.angle = petalRand() * 360.f;
            p.angSpeed = (petalRand() - 0.5f) * 2.4f;
            p.size = 0.18f + petalRand() * 0.31f;
            p.swing = petalRand() * 2.f * PI;
            p.swingSpeed = 0.024f + petalRand() * 0.036f;
            p.swingAmp = 0.0012f + petalRand() * 0.0018f;
            p.colorType = isWhite ? 1 : 0; // new colour type
        }
    }
}

// ================================================================
//  UPDATE ROAD TRAFFIC — moves cars and pedestrians         [Emad]
//
//  Runs every tick (called from update() in main).  Each car and
//  pedestrian moves a tiny step on its own lane / row.  When one
//  goes off the right edge of the canvas it reappears on the
//  left edge (and vice versa for the ones going left).
// ================================================================
static void updateTraffic()
{
    // ---- Move every car ----
    for (int i = 0; i < 8; i++)
    {
        if (carLane[i] == 0) // top lane = going RIGHT
        {
            carX[i] += 0.0030f * carSpeedMult; // dynamic translation speed [Emad]
            if (carX[i] > 1.10f)               // gone off the right edge?
            {
                carX[i] = -1.10f; // wrap back to the left edge
            }
        }
        else // bottom lane = going LEFT
        {
            carX[i] -= 0.0028f * carSpeedMult; // dynamic translation speed [Emad]
            if (carX[i] < -1.10f)              // gone off the left edge?
            {
                carX[i] = 1.10f; // wrap back to the right edge
            }
        }
    }

    // ---- Move every pedestrian ----
    for (int i = 0; i < 6; i++)
    {
        if (pedRow[i] == 0) // top kerb = going RIGHT
        {
            pedX[i] += 0.0017f;
            if (pedX[i] > 1.10f)
            {
                pedX[i] = -1.10f;
            }
        }
        else // bottom kerb = going LEFT
        {
            pedX[i] -= 0.0015f;
            if (pedX[i] < -1.10f)
            {
                pedX[i] = 1.10f;
            }
        }
    }
}

// ================================================================
//  FLOWER BEDS — around the football field edges
// ================================================================
static void drawFlowerBed(float cx, float cy, float rx, float ry, unsigned char pr, unsigned char pg, unsigned char pb)
{
    // Soil base
    diskFan(cx, cy, rx, ry, 18, 92, 62, 28);

    // Green groundcover foliage
    int nClumps = 7;
    for (int c = 0; c < nClumps; c++)
    {
        float t = (float)c / nClumps;
        float fx = cx - rx * 0.85f + t * rx * 1.70f;
        float fy = cy + ((c % 2 == 0) ? -ry * 0.20f : ry * 0.15f);
        float fRx = rx * 0.22f, fRy = ry * 0.50f;
        diskFan(fx, fy, fRx, fRy, 12, 42, 118, 32);
    }

    // Flower blooms — scattered across the bed
    struct Bloom
    {
        float ox, oy;
        unsigned char r, g, b;
        float sz;
    };
    Bloom blooms[] = {
        {-rx * 0.70f, ry * 0.30f, pr, pg, pb, 1.00f},
        {-rx * 0.40f, ry * 0.55f, 255, 220, 80, 0.85f}, // yellow
        {-rx * 0.10f, ry * 0.35f, pr, pg, pb, 0.90f},
        {rx * 0.18f, ry * 0.60f, 255, 255, 255, 0.80f}, // white
        {rx * 0.45f, ry * 0.40f, pr, pg, pb, 1.00f},
        {rx * 0.72f, ry * 0.28f, 255, 150, 60, 0.88f}, // orange
        {-rx * 0.55f, -ry * 0.10f, 255, 255, 255, 0.75f},
        {rx * 0.28f, -ry * 0.05f, pr, pg, pb, 0.82f},
        {rx * 0.60f, ry * 0.15f, 255, 220, 80, 0.78f},
        {-rx * 0.22f, ry * 0.18f, 255, 150, 60, 0.90f},
    };
    int nB = 10;

    for (int i = 0; i < nB; i++)
    {
        float fx = cx + blooms[i].ox;
        float fy = cy + blooms[i].oy;
        float sz = blooms[i].sz * 0.0055f;

        // Stem
        glColor3ub(38, 112, 28);
        glLineWidth(0.9f);
        glBegin(GL_LINES);
        glVertex2f(fx, fy - sz * 1.8f);
        glVertex2f(fx, fy);
        glEnd();

        // 5-petal flower
        for (int p = 0; p < 5; p++)
        {
            float pa = p * 2.f * PI / 5.f;
            float pcx = fx + cosf(pa) * sz * 0.80f;
            float pcy = fy + sinf(pa) * sz * 0.80f;
            glColor4ub(blooms[i].r, blooms[i].g, blooms[i].b, 220);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(pcx, pcy);
            for (int k = 0; k <= 10; k++)
            {
                float a = k * 2.f * PI / 10.f;
                glVertex2f(pcx + cosf(a) * sz, pcy + sinf(a) * sz * 0.85f);
            }
            glEnd();
        }

        // Yellow stamen centre
        glColor3ub(255, 230, 50);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(fx, fy);
        for (int k = 0; k <= 10; k++)
        {
            float a = k * 2.f * PI / 10.f;
            glVertex2f(fx + cosf(a) * sz * 0.35f, fy + sinf(a) * sz * 0.35f);
        }
        glEnd();
    }

    // Border edging stones
    glColor3ub(155, 148, 138);
    glLineWidth(1.4f);
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k <= 30; k++)
    {
        float a = k * 2.f * PI / 30.f;
        glVertex2f(cx + cosf(a) * rx, cy + sinf(a) * ry);
    }
    glEnd();
}

static void drawFootballFieldFlowerBeds()
{
    // Shared Y extents — must match drawRoadAndPlayground()
    // fieldY1 = roadY2 + 0.012 = (-0.72 + 0.012) + 0.012 = -0.708
    // fieldY2 = fieldY1 + 0.335
    const float fieldY1 = -0.708f;
    const float fieldY2 = fieldY1 + 0.335f;
    const float fldX1 = 0.35f;
    const float fldX2 = 0.95f;
    const float fldCX = (fldX1 + fldX2) * 0.5f;

    // ── TOP EDGE  (above the field boundary line) ──
    // Three oval beds nestled just outside the top touchline
    float topBedY = fieldY2 + 0.022f;
    drawFlowerBed(fldX1 + 0.08f, topBedY, 0.055f, 0.018f, 220, 80, 160); // magenta-left
    drawFlowerBed(fldCX, topBedY, 0.062f, 0.020f, 180, 90, 210);         // violet-centre
    drawFlowerBed(fldX2 - 0.08f, topBedY, 0.055f, 0.018f, 220, 80, 160); // magenta-right

    // ── BOTTOM EDGE  (below the field, above brick footpath) ──
    // Three beds mirroring the top row
    float botBedY = fieldY1 - 0.022f;
    drawFlowerBed(fldX1 + 0.08f, botBedY, 0.055f, 0.018f, 230, 100, 60); // coral-left
    drawFlowerBed(fldCX, botBedY, 0.062f, 0.020f, 240, 180, 40);         // golden-centre
    drawFlowerBed(fldX2 - 0.08f, botBedY, 0.055f, 0.018f, 230, 100, 60); // coral-right

    // ── CORNERS — small accent beds at all four field corners ──
    float cRx = 0.030f, cRy = 0.016f;
    drawFlowerBed(fldX1 + 0.032f, fieldY1 - 0.016f, cRx, cRy, 180, 220, 80);  // lime BL
    drawFlowerBed(fldX2 - 0.032f, fieldY1 - 0.016f, cRx, cRy, 180, 220, 80);  // lime BR
    drawFlowerBed(fldX1 + 0.032f, fieldY2 + 0.016f, cRx, cRy, 100, 180, 240); // sky TL
    drawFlowerBed(fldX2 - 0.032f, fieldY2 + 0.016f, cRx, cRy, 100, 180, 240); // sky TR
}

// ================================================================
//  CARS ON THE MAIN ROAD                                   [Emad]
//
//  Two lanes of cars run across the road:
//    Top lane    (y ≈ -0.800) → cars going RIGHT
//    Bottom lane (y ≈ -0.920) → cars going LEFT
//
//  Each car is drawn from a side-on view: a coloured rectangle
//  body, a smaller darker rectangle on top for the cabin, two
//  windows, two big wheels and two small headlights.
//
//  No fancy palette table — colours are picked with a simple
//  switch on a small colour code (0..5).  Easy to read, easy
//  to change, and the same six colours are reused for both
//  cars and pedestrians.
// ================================================================

// ----------------------------------------------------------------
//  pickCarColor  — given a colour code 0..5, fills the three
//  colour values (body, cabin, window).  This replaces the old
//  carPalette[][] array — it is just an if/else ladder so the
//  code reads top-to-bottom.
// ----------------------------------------------------------------
static void pickCarColor(int code,
                         unsigned char &br, unsigned char &bg, unsigned char &bb,
                         unsigned char &rr, unsigned char &rg, unsigned char &rb,
                         unsigned char &wr, unsigned char &wg, unsigned char &wb)
{
    if (code == 0) // RED
    {
        br = 200;
        bg = 40;
        bb = 40;
        rr = 140;
        rg = 20;
        rb = 20;
        wr = 80;
        wg = 140;
        wb = 200;
    }
    else if (code == 1) // BLUE
    {
        br = 40;
        bg = 90;
        bb = 200;
        rr = 25;
        rg = 55;
        rb = 140;
        wr = 90;
        wg = 170;
        wb = 230;
    }
    else if (code == 2) // YELLOW
    {
        br = 220;
        bg = 160;
        bb = 30;
        rr = 155;
        rg = 110;
        rb = 20;
        wr = 100;
        wg = 160;
        wb = 200;
    }
    else if (code == 3) // GREEN
    {
        br = 50;
        bg = 160;
        bb = 60;
        rr = 30;
        rg = 100;
        rb = 38;
        wr = 80;
        wg = 170;
        wb = 210;
    }
    else if (code == 4) // SILVER
    {
        br = 180;
        bg = 180;
        bb = 185;
        rr = 120;
        rg = 120;
        rb = 125;
        wr = 90;
        wg = 155;
        wb = 210;
    }
    else // BLACK (code 5)
    {
        br = 40;
        bg = 40;
        bb = 42;
        rr = 22;
        rg = 22;
        rb = 24;
        wr = 60;
        wg = 110;
        wb = 170;
    }
}

// ----------------------------------------------------------------
//  drawCar  — draws ONE car at world position (cx, cy).
//
//    cx, cy        : centre of the car in world coordinates
//    bw, bh        : half-width and half-height of the car
//    colorCode     : 0..5 (see pickCarColor above)
//    facingRight   : true if the car is going right, else left.
//                    The headlights / front windscreen flip side
//                    based on this so it always looks correct.
// ----------------------------------------------------------------
static void drawCar(float cx, float cy, float bw, float bh, int colorCode, bool facingRight)
{
    // Look up the three colour parts for this car
    unsigned char bodyR, bodyG, bodyB;
    unsigned char cabR, cabG, cabB;
    unsigned char winR, winG, winB;
    pickCarColor(colorCode, bodyR, bodyG, bodyB, cabR, cabG, cabB, winR, winG, winB);

    // sx flips the front/back side of the car when it faces left
    float sx;
    if (facingRight)
    {
        sx = 1.0f;
    }
    else
    {
        sx = -1.0f;
    }

    // -------- Underbody / wheel arch (dark thin slab below body) --------
    glColor3ub(30, 30, 32);
    glBegin(GL_QUADS);
    glVertex2f(cx - bw, cy - bh * 0.10f);
    glVertex2f(cx + bw, cy - bh * 0.10f);
    glVertex2f(cx + bw * 0.92f, cy - bh * 0.30f);
    glVertex2f(cx - bw * 0.92f, cy - bh * 0.30f);
    glEnd();

    // -------- Main body (the big coloured rectangle) --------
    glColor3ub(bodyR, bodyG, bodyB);
    glBegin(GL_QUADS);
    glVertex2f(cx - bw, cy - bh * 0.10f);
    glVertex2f(cx + bw, cy - bh * 0.10f);
    glVertex2f(cx + bw, cy + bh * 0.30f);
    glVertex2f(cx - bw, cy + bh * 0.30f);
    glEnd();

    // -------- Cabin (smaller darker rectangle on top) --------
    glColor3ub(cabR, cabG, cabB);
    glBegin(GL_QUADS);
    glVertex2f(cx - bw * 0.60f, cy + bh * 0.28f);
    glVertex2f(cx + bw * 0.60f, cy + bh * 0.28f);
    glVertex2f(cx + bw * 0.52f, cy + bh * 0.72f);
    glVertex2f(cx - bw * 0.52f, cy + bh * 0.72f);
    glEnd();

    // -------- Front windscreen (the "front" side flips with sx) --------
    glColor3ub(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(cx + sx * bw * 0.06f, cy + bh * 0.30f);
    glVertex2f(cx + sx * bw * 0.50f, cy + bh * 0.30f);
    glVertex2f(cx + sx * bw * 0.46f, cy + bh * 0.68f);
    glVertex2f(cx + sx * bw * 0.08f, cy + bh * 0.68f);
    glEnd();

    // -------- Rear windscreen (opposite side of the front) --------
    glColor3ub(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(cx - sx * bw * 0.06f, cy + bh * 0.30f);
    glVertex2f(cx - sx * bw * 0.50f, cy + bh * 0.30f);
    glVertex2f(cx - sx * bw * 0.46f, cy + bh * 0.68f);
    glVertex2f(cx - sx * bw * 0.08f, cy + bh * 0.68f);
    glEnd();

    // -------- Headlights and tail lights (front + back) --------
    diskFan(cx + sx * bw * 0.76f, cy - bh * 0.02f, bw * 0.14f, bh * 0.10f, 10,
            255, 248, 200); // headlight (front)
    diskFan(cx - sx * bw * 0.76f, cy - bh * 0.02f, bw * 0.12f, bh * 0.09f, 10,
            220, 30, 30); // tail-light (back)

    // -------- Two big wheels with lighter rims in the centre --------
    float wy = cy - bh * 0.22f;
    float wr = bh * 0.18f;
    diskFan(cx + bw * 0.62f, wy, wr, wr * 0.55f, 14, 22, 22, 24);            // tyre R
    diskFan(cx - bw * 0.62f, wy, wr, wr * 0.55f, 14, 22, 22, 24);            // tyre L
    diskFan(cx + bw * 0.62f, wy, wr * 0.55f, wr * 0.30f, 10, 160, 160, 168); // rim R
    diskFan(cx - bw * 0.62f, wy, wr * 0.55f, wr * 0.30f, 10, 160, 160, 168); // rim L
}

// ================================================================
//  PEDESTRIANS ON THE FOOTPATH                              [Emad]
//
//  Each pedestrian is a small stick figure: shirt rectangle,
//  trouser rectangle, head circle.  No walking animation —
//  they slide along the footpath in a straight line.  The
//  arms are NOT drawn (kept simple on purpose).
//
//  Six pedestrians total, half on the top kerb walking right,
//  half on the bottom kerb walking left.
// ================================================================

// ----------------------------------------------------------------
//  pickPedColor  — same idea as pickCarColor: a simple if-else
//  ladder that fills shirt / trousers / skin colour from a
//  small code 0..5.
// ----------------------------------------------------------------
static void pickPedColor(int code,
                         unsigned char &sr, unsigned char &sg, unsigned char &sb,
                         unsigned char &tr, unsigned char &tg, unsigned char &tb,
                         unsigned char &kr, unsigned char &kg, unsigned char &kb)
{
    if (code == 0) // RED shirt
    {
        sr = 220;
        sg = 60;
        sb = 60;
        tr = 50;
        tg = 70;
        tb = 120;
        kr = 220;
        kg = 175;
        kb = 130;
    }
    else if (code == 1) // BLUE shirt
    {
        sr = 60;
        sg = 100;
        sb = 220;
        tr = 30;
        tg = 30;
        tb = 30;
        kr = 200;
        kg = 155;
        kb = 110;
    }
    else if (code == 2) // GREEN shirt
    {
        sr = 50;
        sg = 170;
        sb = 80;
        tr = 60;
        tg = 40;
        tb = 20;
        kr = 235;
        kg = 190;
        kb = 145;
    }
    else if (code == 3) // YELLOW shirt
    {
        sr = 220;
        sg = 180;
        sb = 60;
        tr = 80;
        tg = 80;
        tb = 80;
        kr = 180;
        kg = 130;
        kb = 90;
    }
    else if (code == 4) // PURPLE shirt
    {
        sr = 180;
        sg = 60;
        sb = 180;
        tr = 40;
        tg = 40;
        tb = 60;
        kr = 225;
        kg = 180;
        kb = 135;
    }
    else // WHITE shirt (code 5)
    {
        sr = 240;
        sg = 240;
        sb = 240;
        tr = 30;
        tg = 50;
        tb = 90;
        kr = 215;
        kg = 170;
        kb = 125;
    }
}

// ----------------------------------------------------------------
//  drawPedestrian  — draws ONE pedestrian at world (cx, cy).
//  No animation — body parts are at fixed offsets.
// ----------------------------------------------------------------
static void drawPedestrian(float cx, float cy, int colorCode)
{
    // Look up the three colour parts for this pedestrian
    unsigned char shirtR, shirtG, shirtB;
    unsigned char trouR, trouG, trouB;
    unsigned char skinR, skinG, skinB;
    pickPedColor(colorCode, shirtR, shirtG, shirtB,
                 trouR, trouG, trouB,
                 skinR, skinG, skinB);

    const float W = 0.012f; // half-body width
    const float H = 0.052f; // total body height

    // -------- Trousers (two short dark rectangles for the legs) --------
    glColor3ub(trouR, trouG, trouB);
    // left leg
    glBegin(GL_QUADS);
    glVertex2f(cx - W * 0.40f, cy + H * 0.01f);
    glVertex2f(cx - W * 0.05f, cy + H * 0.01f);
    glVertex2f(cx - W * 0.05f, cy + H * 0.42f);
    glVertex2f(cx - W * 0.40f, cy + H * 0.42f);
    glEnd();
    // right leg
    glBegin(GL_QUADS);
    glVertex2f(cx + W * 0.05f, cy + H * 0.01f);
    glVertex2f(cx + W * 0.40f, cy + H * 0.01f);
    glVertex2f(cx + W * 0.40f, cy + H * 0.42f);
    glVertex2f(cx + W * 0.05f, cy + H * 0.42f);
    glEnd();

    // -------- Shirt / torso (coloured rectangle above the legs) --------
    glColor3ub(shirtR, shirtG, shirtB);
    glBegin(GL_QUADS);
    glVertex2f(cx - W * 0.75f, cy + H * 0.40f);
    glVertex2f(cx + W * 0.75f, cy + H * 0.40f);
    glVertex2f(cx + W * 0.62f, cy + H * 0.72f);
    glVertex2f(cx - W * 0.62f, cy + H * 0.72f);
    glEnd();

    // -------- Neck + head --------
    glColor3ub(skinR, skinG, skinB);
    glBegin(GL_QUADS); // small neck rect
    glVertex2f(cx - W * 0.22f, cy + H * 0.70f);
    glVertex2f(cx + W * 0.22f, cy + H * 0.70f);
    glVertex2f(cx + W * 0.22f, cy + H * 0.78f);
    glVertex2f(cx - W * 0.22f, cy + H * 0.78f);
    glEnd();
    diskFan(cx, cy + H * 0.90f, W * 0.44f, W * 0.48f, 14,
            skinR, skinG, skinB); // round head

    // -------- Hair (small dark ellipse on top of the head) --------
    diskFan(cx - W * 0.08f, cy + H * 0.96f, W * 0.46f, W * 0.30f, 12,
            40, 28, 12);

    // -------- Umbrella (only in rain mode) --------
    if (rainMode)
    {
        float ux = cx;
        float uy = cy + H * 1.10f;  // just above the head
        float ur = W * 2.0f;        // umbrella radius
        float uTop = uy + W * 1.5f; // top of the dome

        // Canopy — filled dome (triangle fan half-circle)
        glColor3ub(220, 50, 50); // red umbrella
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(ux, uy);
        for (int s = 0; s <= 18; s++)
        {
            float a = (float)s / 18.0f * PI; // 0 → PI (top half only)
            glVertex2f(ux + cosf(a) * ur, uy + sinf(a) * W * 1.5f);
        }
        glEnd();

        // Canopy outline
        glColor3ub(160, 20, 20);
        glLineWidth(1.5f);
        glBegin(GL_LINE_STRIP);
        for (int s = 0; s <= 18; s++)
        {
            float a = (float)s / 18.0f * PI;
            glVertex2f(ux + cosf(a) * ur, uy + sinf(a) * W * 1.5f);
        }
        glEnd();

        // Little scallop tips along the bottom edge
        glColor3ub(190, 30, 30);
        for (int s = 0; s <= 4; s++)
        {
            float a = (float)s / 4.0f * PI;
            float tx = ux + cosf(a) * ur;
            float ty = uy;
            diskFan(tx, ty, W * 0.22f, W * 0.22f, 8, 200, 40, 40);
        }

        // Handle — thin dark curved stick
        glColor3ub(60, 35, 10);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        glVertex2f(ux, uy);
        glVertex2f(ux, cy + H * 0.75f);            // straight down to shoulder
        glVertex2f(ux + W * 0.8f, cy + H * 0.65f); // slight curve outward
        glEnd();
        glLineWidth(1.0f);
    }
}

// ----------------------------------------------------------------
//  drawTraffic  — draws all 8 cars by reading carX[], carLane[],
//  carColor[].  carLane[i] decides the Y position and direction.
// ----------------------------------------------------------------
static void drawTraffic()
{
    for (int i = 0; i < 8; i++)
    {
        // decide Y and facing direction from the lane
        float carY;
        bool facingRight;
        if (carLane[i] == 0) // top lane = going right
        {
            carY = -0.800f;
            facingRight = true;
        }
        else // bottom lane = going left
        {
            carY = -0.920f;
            facingRight = false;
        }

        drawCar(carX[i], carY, 0.075f, 0.058f,
                carColor[i], facingRight);
    }
}

// ----------------------------------------------------------------
//  drawPedestrians  — draws all 6 pedestrians.  Y comes from the
//  row they walk on (top kerb or bottom kerb).
// ----------------------------------------------------------------
static void drawPedestrians()
{
    for (int i = 0; i < 6; i++)
    {
        float y;
        if (pedRow[i] == 0)
        {
            y = -0.712f; // top kerb
        }
        else
        {
            y = -0.968f; // bottom kerb
        }
        drawPedestrian(pedX[i], y, pedColor[i]);
    }
}

// ----------------------------------------------------------------
//  initRoadTraffic  — gives starting positions to the 8 cars and
//  6 pedestrians.  Called once at program start.  Replaces all
//  the old initialisation loops that used to live in init().
// ----------------------------------------------------------------
static void initRoadTraffic()
{
    // Cars — 4 on the top lane (going right), 4 on the bottom (going left).
    // Spread them along the road so they don't all bunch together.
    carX[0] = -0.90f;
    carLane[0] = 0;
    carColor[0] = 0; // red,    top
    carX[1] = -0.40f;
    carLane[1] = 1;
    carColor[1] = 1; // blue,   bottom
    carX[2] = 0.10f;
    carLane[2] = 0;
    carColor[2] = 2; // yellow, top
    carX[3] = 0.60f;
    carLane[3] = 1;
    carColor[3] = 3; // green,  bottom
    carX[4] = -0.65f;
    carLane[4] = 0;
    carColor[4] = 4; // silver, top
    carX[5] = -0.15f;
    carLane[5] = 1;
    carColor[5] = 5; // black,  bottom
    carX[6] = 0.35f;
    carLane[6] = 0;
    carColor[6] = 0; // red,    top
    carX[7] = 0.85f;
    carLane[7] = 1;
    carColor[7] = 2; // yellow, bottom

    // Pedestrians — 3 on the top kerb (going right), 3 on the bottom (left).
    pedX[0] = -0.80f;
    pedRow[0] = 0;
    pedColor[0] = 0;
    pedX[1] = -0.10f;
    pedRow[1] = 1;
    pedColor[1] = 1;
    pedX[2] = 0.55f;
    pedRow[2] = 0;
    pedColor[2] = 2;
    pedX[3] = 0.20f;
    pedRow[3] = 1;
    pedColor[3] = 3;
    pedX[4] = -0.45f;
    pedRow[4] = 0;
    pedColor[4] = 4;
    pedX[5] = 0.75f;
    pedRow[5] = 1;
    pedColor[5] = 5;
}

// ================================================================
//  AUTUMN SEASON — falling leaves                          [Emad]
//
//  Every tick we move
//  each leaf down a bit. When a leaf goes off
//  the bottom, it respawns at the top.
// ================================================================

// ----------------------------------------------------------------
//  initAutumn  — give every leaf a random starting position,
//  fall speed, and one of four autumn colours.
// ----------------------------------------------------------------
static void initAutumn() // [Emad]
{
    for (int i = 0; i < MAX_LEAVES; i++)
    {
        leafX[i] = -1.0f + (rand() % 200) / 100.0f;      // -1 .. +1
        leafY[i] = -1.0f + (rand() % 200) / 100.0f;      // -1 .. +1
        leafSpeed[i] = 0.003f + (rand() % 10) / 2000.0f; // slow fall
        leafColor[i] = rand() % 4;                       // 0..3

        // ── Rotation state for dynamic glRotatef() (Emad) ──
        // Random starting angle so leaves don't all face the same way,
        // and a random spin speed (in deg/tick) in the range -3..+3 so
        // some leaves rotate clockwise, some counter-clockwise, and a
        // few barely rotate at all (= more realistic).
        leafAngle[i] = (float)(rand() % 360);                    // 0..359 deg
        leafSpin[i] = (((rand() % 200) / 100.0f) - 1.0f) * 3.0f; // -3..+3 deg/tick
    }
}

// ----------------------------------------------------------------
//  drawAutumn  — draw all the leaves.  Each leaf is one small
//  diamond shape (4 triangle vertices) in an autumn colour.
// ----------------------------------------------------------------
static void drawAutumn() // [Emad]
{
    for (int i = 0; i < MAX_LEAVES; i++)
    {
        // Pick the colour for this leaf
        if (leafColor[i] == 0)
        {
            glColor3ub(220, 110, 30); // orange
        }
        else if (leafColor[i] == 1)
        {
            glColor3ub(230, 180, 40); // yellow
        }
        else if (leafColor[i] == 2)
        {
            glColor3ub(140, 70, 30); // brown
        }
        else
        {
            glColor3ub(180, 40, 30); // red
        }

        // ── Dynamic transformations via OpenGL matrix stack (Emad) ──
        // Order is intentional and important:
        //   1) glTranslatef : move the origin to where the leaf actually is
        //   2) glRotatef    : spin the leaf around its own centre (Z axis)
        //   3) glScalef     : grow/shrink the leaf (user-controlled by k/l)
        // The leaf shape itself is drawn AROUND THE ORIGIN (0,0), so that
        // rotation and scaling pivot around the leaf's centre — not around
        // the world origin.  glPushMatrix/glPopMatrix isolates these
        // transforms so they don't leak into anything else drawn after.
        glPushMatrix();
        glTranslatef(leafX[i], leafY[i], 0.0f);    // dynamic translation
        glRotatef(leafAngle[i], 0.0f, 0.0f, 1.0f); // dynamic rotation (animated)
        glScalef(leafScale, leafScale, 1.0f);      // dynamic scaling (key-controlled)

        // Diamond drawn around origin (matrix handles the actual placement)
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.012f);  // top
        glVertex2f(0.010f, 0.0f);  // right
        glVertex2f(0.0f, -0.012f); // bottom
        glVertex2f(-0.010f, 0.0f); // left
        glEnd();
        glPopMatrix();
    }
}

// ----------------------------------------------------------------
//  updateAutumn  — fall.  Move every leaf down by its
//  speed, and respawn at the top if it
//  has fallen off the bottom of the screen.
// ----------------------------------------------------------------
static void updateAutumn() // [Emad]
{
    for (int i = 0; i < MAX_LEAVES; i++)
    {
        leafY[i] -= leafSpeed[i]; // fall down

        // ── Advance per-leaf rotation angle (Emad) ──
        // Each leaf has its own spin speed (deg/tick), so they all rotate
        // independently — some clockwise, some counter-clockwise.  Wrap
        // the angle to keep it bounded; glRotatef handles any value but
        // wrapping avoids float drift over very long sessions.
        leafAngle[i] += leafSpin[i];
        if (leafAngle[i] >= 360.0f)
            leafAngle[i] -= 360.0f;
        if (leafAngle[i] < 0.0f)
            leafAngle[i] += 360.0f;

        if (leafY[i] < -1.0f)
        {
            leafY[i] = 1.0f;                            // back to top
            leafX[i] = -1.0f + (rand() % 200) / 100.0f; // new X
        }
    }
}

// ----------------------------------------------------------------
//  drawAutumnSky  — paints the sky in warm autumn tones (orange
//  at the top, golden at the horizon).  Darkens at night so the
//  day/night cycle still works while autumn is active.
// ----------------------------------------------------------------
static void drawAutumnSky() // [Emad]
{
    float db = getDayBlend(); // 1 = day, 0 = night
    float night = 1.0f - db;  // 1 = night, 0 = day

    // Top of the sky — deep orange in the day, dark navy at night
    float topR = lerpF(220.0f, 18.0f, night);
    float topG = lerpF(95.0f, 14.0f, night);
    float topB = lerpF(50.0f, 28.0f, night);

    // Bottom of the sky — warm golden glow during the day
    float botR = lerpF(245.0f, 30.0f, night);
    float botG = lerpF(160.0f, 20.0f, night);
    float botB = lerpF(70.0f, 32.0f, night);

    glBegin(GL_QUADS);
    glColor3f(topR / 255.0f, topG / 255.0f, topB / 255.0f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(botR / 255.0f, botG / 255.0f, botB / 255.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

// ----------------------------------------------------------------
//  drawAutumnGround  — paints the ground in browner / drier tones
//  to match the season.  Same shape as Prottoy's drawDayNightGround.
// ----------------------------------------------------------------
static void drawAutumnGround() // [Emad]
{
    float db = getDayBlend();
    float night = 1.0f - db;

    // Dry brown grass during the day, much darker at night
    float gr = lerpF(0.55f, 0.10f, night);
    float gg = lerpF(0.35f, 0.07f, night);
    float gb = lerpF(0.12f, 0.05f, night);

    glColor3f(gr, gg, gb);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, -0.15f);
    glVertex2f(-1.0f, -0.15f);
    glEnd();
}

// ----------------------------------------------------------------
//  drawAutumnTint  — translucent orange overlay on top of the
//  whole canvas.  This is what gives the trees and buildings
//  their warm autumn look without rewriting any of those
//  drawing functions.  Drawn AFTER trees but BEFORE buildings
//  so trees pick up the colour shift.
// ----------------------------------------------------------------
static void drawAutumnTint() // [Emad]
{
    glColor4ub(220, 110, 30, 55); // soft orange, low alpha
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
}

//  WINTER SEASON — falling snow [Shajmin]
//
//  Pool-of-particles pattern, but white round dots instead of blue lines,
//  and slightly slower so the snow looks like it's gently drifting.

//  initWinter  — random starting position and speed for each snowflake.
static void initWinter() // [Shajmin]
{
    for (int i = 0; i < MAX_SNOW; i++)
    {
        snowX[i] = -1.0f + (rand() % 200) / 100.0f;
        snowY[i] = -1.0f + (rand() % 200) / 100.0f;
        snowSpeed[i] = 0.004f + (rand() % 10) / 2000.0f; // slow fall
    }
}

//  drawWinter  — draw all the snowflakes as tiny white squares.
static void drawWinter() // [Shajmin]
{
    glColor3ub(255, 255, 255); // white snow
    for (int i = 0; i < MAX_SNOW; i++)
    {
        // To make the flake size adjustable at runtime,
        // we translate the local origin to the flake's position, then
        // glScalef() by snowScale, then draw the disk at the origin.
        // Press 'v' to shrink every flake, 'b' to grow them.
        glPushMatrix();
        glTranslatef(snowX[i], snowY[i], 0.0f);
        glScalef(snowScale, snowScale, 1.0f); // [Shajmin]
        diskFan(0.0f, 0.0f, 0.006f, 0.006f, 8, 255, 255, 255);
        glPopMatrix();
    }
}

//  updateWinter  — fall down. Respawn at the top when off-screen.
static void updateWinter() // [Shajmin]
{
    for (int i = 0; i < MAX_SNOW; i++)
    {
        snowY[i] -= snowSpeed[i];
        if (snowY[i] < -1.0f)
        {
            snowY[i] = 1.0f;
            snowX[i] = -1.0f + (rand() % 200) / 100.0f;
        }
    }
}

//  drawWinterSky  — pale grey-white winter sky.  Darkens at
//  night through the same day/night blend used everywhere else.
static void drawWinterSky() // [Shajmin]
{
    float db = getDayBlend();
    float night = 1.0f - db;

    // Top: cold pale blue-grey by day, deep dark blue at night
    float topR = lerpF(180.0f, 10.0f, night);
    float topG = lerpF(195.0f, 14.0f, night);
    float topB = lerpF(210.0f, 30.0f, night);

    // Bottom: very pale white-grey by day
    float botR = lerpF(225.0f, 25.0f, night);
    float botG = lerpF(230.0f, 30.0f, night);
    float botB = lerpF(235.0f, 45.0f, night);

    glBegin(GL_QUADS);
    glColor3f(topR / 255.0f, topG / 255.0f, topB / 255.0f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(botR / 255.0f, botG / 255.0f, botB / 255.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

//  drawWinterGround  — snowy white ground.  Stays whitish even
//  at night (snow reflects light) but dims a bit.
static void drawWinterGround() // [Shajmin]
{
    float db = getDayBlend();
    float night = 1.0f - db;

    // White snow during the day, dim grey-blue at night
    float gr = lerpF(0.92f, 0.18f, night);
    float gg = lerpF(0.94f, 0.22f, night);
    float gb = lerpF(0.96f, 0.30f, night);

    glColor3f(gr, gg, gb);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, -0.15f);
    glVertex2f(-1.0f, -0.15f);
    glEnd();
}

//  drawWinterTint  — soft white overlay on top of the whole
//  canvas.  Gives trees and buildings a "snow has settled"
//  look without touching their code.
static void drawWinterTint() // [Shajmin]
{
    glColor4ub(220, 230, 240, 65); // pale icy blue, low alpha
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
}

//  drawWinterRoadSnow  — paints a translucent white strip on top
//  of the road / footpath area, so the asphalt looks covered in
//  snow.  Drawn AFTER drawRoadAndPlayground so it sits over the
//  asphalt.
static void drawWinterRoadSnow() // [Shajmin]
{
    glColor4ub(245, 248, 252, 200); // mostly opaque white
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, -0.708f); // top of road / bottom of fields
    glVertex2f(-1.0f, -0.708f);
    glEnd();
}

//  MOVING PLAYERS — bounce inside the courts             [Shajmin]
//
//  Set up starting positions and velocities for every player
//  once at program start, and update them every tick.

//  initPlayers  — set the starting position and a small velocity
//  for each player on both fields.
//
//  Court bounds:
//     Basketball : X  -0.10  →  +0.35     Y  -0.708  →  -0.373
//     Football   : X  +0.35  →  +0.95     Y  -0.708  →  -0.373
static void initPlayers() // [Shajmin]
{
    // Basketball players (4 of them, just place them inside the court)
    bballX[0] = -0.060f;
    bballY[0] = -0.600f;
    bballVX[0] = 0.0030f;
    bballVY[0] = 0.0020f;
    bballX[1] = 0.090f;
    bballY[1] = -0.680f;
    bballVX[1] = -0.0025f;
    bballVY[1] = 0.0030f;
    bballX[2] = 0.180f;
    bballY[2] = -0.460f;
    bballVX[2] = 0.0020f;
    bballVY[2] = -0.0025f;
    bballX[3] = 0.280f;
    bballY[3] = -0.520f;
    bballVX[3] = -0.0030f;
    bballVY[3] = -0.0020f;

    // Football players (6 of them, spread across the pitch)
    soccerX[0] = 0.420f;
    soccerY[0] = -0.640f;
    soccerVX[0] = 0.0025f;
    soccerVY[0] = 0.0020f;
    soccerX[1] = 0.540f;
    soccerY[1] = -0.460f;
    soccerVX[1] = -0.0030f;
    soccerVY[1] = 0.0015f;
    soccerX[2] = 0.640f;
    soccerY[2] = -0.580f;
    soccerVX[2] = 0.0020f;
    soccerVY[2] = -0.0025f;
    soccerX[3] = 0.740f;
    soccerY[3] = -0.500f;
    soccerVX[3] = -0.0025f;
    soccerVY[3] = 0.0020f;
    soccerX[4] = 0.830f;
    soccerY[4] = -0.660f;
    soccerVX[4] = 0.0030f;
    soccerVY[4] = 0.0025f;
    soccerX[5] = 0.880f;
    soccerY[5] = -0.420f;
    soccerVX[5] = -0.0020f;
    soccerVY[5] = -0.0030f;
}

//  updatePlayers  — move every player by its velocity, and bounce
//  off the walls of its court. Bouncing means: if the player is
//  about to go past a wall, flip the relevant velocity sign so
//  it heads the other way.
static void updatePlayers() // [Shajmin]
{
    // Basketball court bounds
    // The white outline is drawn at 0.016 inside the court rectangle,
    // so we use those tightened values to keep the players inside it.
    float bbLeft = -0.084f;   // -0.10 + 0.016
    float bbRight = 0.334f;   // +0.35 - 0.016
    float bbBottom = -0.692f; // -0.708 + 0.016
    float bbTop = -0.389f;    // -0.373 - 0.016

    for (int i = 0; i < BBALL_COUNT; i++)
    {
        // 'h' / 'j' keys scale the
        // per-tick displacement without touching the raw velocity, so bouncing still works.
        bballX[i] += bballVX[i] * playerSpeedMult; // [Shajmin]
        bballY[i] += bballVY[i] * playerSpeedMult; // [Shajmin]

        // Bounce off the left or right wall
        if (bballX[i] < bbLeft)
        {
            bballX[i] = bbLeft;
            bballVX[i] = -bballVX[i];
        }
        if (bballX[i] > bbRight)
        {
            bballX[i] = bbRight;
            bballVX[i] = -bballVX[i];
        }

        // Bounce off the top or bottom wall
        if (bballY[i] < bbBottom)
        {
            bballY[i] = bbBottom;
            bballVY[i] = -bballVY[i];
        }
        if (bballY[i] > bbTop)
        {
            bballY[i] = bbTop;
            bballVY[i] = -bballVY[i];
        }
    }

    // ---- Football pitch bounds (the WHITE lines, not the outer edge) ----
    float fbLeft = 0.366f;    // +0.35 + 0.016
    float fbRight = 0.934f;   // +0.95 - 0.016
    float fbBottom = -0.692f; // -0.708 + 0.016
    float fbTop = -0.389f;    // -0.373 - 0.016

    for (int i = 0; i < SOCCER_COUNT; i++)
    {
        soccerX[i] += soccerVX[i] * playerSpeedMult; // [Shajmin]
        soccerY[i] += soccerVY[i] * playerSpeedMult; // [Shajmin]

        if (soccerX[i] < fbLeft)
        {
            soccerX[i] = fbLeft;
            soccerVX[i] = -soccerVX[i];
        }
        if (soccerX[i] > fbRight)
        {
            soccerX[i] = fbRight;
            soccerVX[i] = -soccerVX[i];
        }
        if (soccerY[i] < fbBottom)
        {
            soccerY[i] = fbBottom;
            soccerVY[i] = -soccerVY[i];
        }
        if (soccerY[i] > fbTop)
        {
            soccerY[i] = fbTop;
            soccerVY[i] = -soccerVY[i];
        }
    }
}

//  MOVING PLAYERS ON THE FIELDS                          [Shajmin]
//
//  Players on the basketball court and football pitch that
//  bounce around inside their own field. Each player has its
//  own (X, Y) position and (VX, VY) velocity. Every tick:
//      X += VX,  Y += VY
//  When the player hits the edge of the field, that velocity is
//  flipped so it bounces off the wall.
//  Each player is a tiny figure: shorts rectangle, shirt
//  rectangle, head circle.  Drawn with a small helper so the
//  same code is reused for every player.

//  drawPlayer  — draws ONE standing player at world (x, y).
//
//  shirtR/G/B    : colour of the shirt and arms
//  shortsR/G/B   : colour of the shorts
//  Head colour is a fixed skin tone for all players.
static void drawPlayer(float x, float y,
                       unsigned char shirtR, unsigned char shirtG, unsigned char shirtB,
                       unsigned char shortsR, unsigned char shortsG, unsigned char shortsB)
{
    // Shorts — small dark rectangle (the player's lower body)
    glColor3ub(shortsR, shortsG, shortsB);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.007f, y - 0.006f);
    glVertex2f(x + 0.007f, y - 0.006f);
    glVertex2f(x + 0.007f, y + 0.004f);
    glVertex2f(x - 0.007f, y + 0.004f);
    glEnd();

    // Shirt — coloured rectangle on top of the shorts
    glColor3ub(shirtR, shirtG, shirtB);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.009f, y + 0.002f);
    glVertex2f(x + 0.009f, y + 0.002f);
    glVertex2f(x + 0.009f, y + 0.022f);
    glVertex2f(x - 0.009f, y + 0.022f);
    glEnd();

    // Head — small skin-tone circle just above the shirt
    diskFan(x, y + 0.028f, 0.008f, 0.009f, 10, 215, 175, 128);
}

//  drawSportsPlayers  — places every player on the courts.
//  No loops over arrays: each player is one direct call.
//
//  Basketball court spans X from about -0.10 to +0.35.
//  Football pitch    spans X from about  +0.35 to +0.95.
//  Both use Y from about -0.708 (bottom) to -0.373 (top).
static void drawSportsPlayers() // [Shajmin]
{
    // Basketball court (red team vs blue team)
    // Players 0 and 2 are red, 1 and 3 are blue. Position comes
    // from the bballX[] / bballY[] arrays (moved each tick).
    drawPlayer(bballX[0], bballY[0], 200, 40, 40, 230, 230, 235); // red
    drawPlayer(bballX[1], bballY[1], 38, 70, 190, 230, 230, 235); // blue
    drawPlayer(bballX[2], bballY[2], 200, 40, 40, 230, 230, 235); // red
    drawPlayer(bballX[3], bballY[3], 38, 70, 190, 230, 230, 235); // blue

    // Football pitch (green team vs yellow team)
    // Even index = green, odd = yellow. Position from soccerX[] / soccerY[].
    drawPlayer(soccerX[0], soccerY[0], 38, 140, 48, 22, 22, 24);  // green
    drawPlayer(soccerX[1], soccerY[1], 210, 160, 30, 22, 22, 24); // yellow
    drawPlayer(soccerX[2], soccerY[2], 38, 140, 48, 22, 22, 24);  // green
    drawPlayer(soccerX[3], soccerY[3], 210, 160, 30, 22, 22, 24); // yellow
    drawPlayer(soccerX[4], soccerY[4], 38, 140, 48, 22, 22, 24);  // green
    drawPlayer(soccerX[5], soccerY[5], 210, 160, 30, 22, 22, 24); // yellow
}
// ================================================================
//  [B] drawStars()  — deterministic star field, fades with dawn.
//      Call immediately after drawSkyGradient().
// ================================================================
// Helper to handle the random number generation
// We pass the seed by reference so it updates correctly
static float get_random_float(unsigned int &seed)
{
    seed = seed * 1664525u + 1013904223u;
    return (float)(seed & 0x7FFF) / 32767.0f;
}

static void drawStars()
{
    float dl = getDaylight(); // Assuming this is defined elsewhere

    // Stars visible when daylight < 0.35; fully out below 0.10
    float alpha = 1.0f - (dl / 0.35f);
    if (alpha <= 0.0f)
        return;
    if (alpha > 1.0f)
        alpha = 1.0f;

    // Reset seed for the first batch of stars
    unsigned int seed = 0xABCD1234u;

    glEnable(GL_POINT_SMOOTH);
    glPointSize(1.6f);
    glBegin(GL_POINTS);
    for (int s = 0; s < 220; s++)
    {
        float px = get_random_float(seed) * 2.0f - 1.0f;
        float py = get_random_float(seed) * 0.92f + 0.08f; // above the horizon strip
        float br = 0.55f + get_random_float(seed) * 0.45f; // per-star brightness
        glColor4f(br, br * 0.97f, br, alpha * br);
        glVertex2f(px, py);
    }
    glEnd();

    // A few slightly larger bright stars
    glPointSize(2.4f);

    // Reset seed to a different value for predictable "larger" stars
    seed = 0xFEEDF00Du;

    glBegin(GL_POINTS);
    for (int s = 0; s < 28; s++)
    {
        float px = get_random_float(seed) * 2.0f - 1.0f;
        float py = get_random_float(seed) * 0.85f + 0.12f;
        glColor4f(1.0f, 0.98f, 0.90f, alpha * 0.85f);
        glVertex2f(px, py);
    }
    glEnd();

    glPointSize(1.0f);
    glDisable(GL_POINT_SMOOTH);
}

// ================================================================
//  [C] drawSunAt(sx, sy) //[Prottoy] with summer intensity boost
// ================================================================
static void drawSunAt(float sx, float sy)
{
    float asp = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);

    // Fade sun out near the horizon so it doesn't pop
    float dl = getDaylight();
    float alpha = fminf(dl * 3.5f, 1.0f); // full opacity once past dawn
    if (alpha <= 0.f)                     // adjusting the transparency of the sun during sunrise
        return;

    // Zidane - Summer Season: intensify sun during summer days
    float intensity = 1.0f;
    if (summerMode && isDay)
    {
        intensity = 1.4f;
    }

    int segments = 48;

    // 1. Outer atmospheric halo
    float outerR = 0.088f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(255, 200, 60, (unsigned char)(180 * alpha * intensity));
    glVertex2f(sx, sy);
    for (int i = 0; i <= segments; i++)
    {
        float a = i * 2.f * PI / segments;
        glColor4ub(255, 200, 60, 0);
        glVertex2f(sx + cosf(a) * outerR / asp, sy + sinf(a) * outerR);
    }
    glEnd();

    // 2. Mid glow
    float midR = 0.076f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(255, 215, 90, (unsigned char)(220 * alpha * intensity));
    glVertex2f(sx, sy);
    for (int i = 0; i <= segments; i++)
    {
        float a = i * 2.f * PI / segments;
        glColor4ub(255, 215, 90, 0);
        glVertex2f(sx + cosf(a) * midR / asp, sy + sinf(a) * midR);
    }
    glEnd();

    // 3. Solid corona
    float coronaR = 0.040f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(255, 242, 160, (unsigned char)(230 * alpha * intensity));
    glVertex2f(sx, sy);
    for (int i = 0; i <= segments; i++)
    {
        float a = i * 2.f * PI / segments;
        glColor4ub(255, 242, 160, (unsigned char)(30 * alpha));
        glVertex2f(sx + cosf(a) * coronaR / asp, sy + sinf(a) * coronaR);
    }
    glEnd();

    // 4. Bright core
    float coreR = 0.024f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(255, 255, 252, (unsigned char)(250 * alpha * intensity));
    glVertex2f(sx, sy);
    for (int i = 0; i <= segments; i++)
    {
        float a = i * 2.f * PI / segments;
        unsigned char r = (unsigned char)(255);
        unsigned char g = (unsigned char)(255 - 17 * (i / (float)segments));
        unsigned char b = (unsigned char)(252 - 112 * (i / (float)segments));
        glColor4ub(r, g, b, 0);
        glVertex2f(sx + cosf(a) * coreR / asp, sy + sinf(a) * coreR);
    }
    glEnd();

    // 5. Specular hot-spot
    float spotR = 0.012f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(255, 255, 255, (unsigned char)(200 * alpha * intensity));
    glVertex2f(sx - 0.007f / asp, sy + 0.009f);
    for (int i = 0; i <= 20; i++)
    {
        float a = i * 2.f * PI / 20;
        glColor4ub(255, 255, 255, 0);
        glVertex2f(sx - 0.007f / asp + cosf(a) * spotR / asp,
                   sy + 0.009f + sinf(a) * spotR);
    }
    glEnd();
}

// ================================================================
//  [C] drawMoon()  — crescent moon, visible only at night.
// ================================================================
static void drawMoon()
{
    if (isDay)
        return;
    float mx, my;
    getMoonPos(mx, my);
    if (my < -0.05f)
        return; // below horizon, skip

    float asp = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);
    float alpha = isDay ? fmaxf(0.f, 1.f - getDaylight() / 0.15f) : 1.0f; // creating the fadding appearance of the moon
    const int SEG = 64;

    // Soft outer glow
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(180, 195, 255, (unsigned char)(38 * alpha));
    glVertex2f(mx, my);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glColor4ub(180, 195, 255, 0);
        glVertex2f(mx + cosf(a) * 0.075f / asp, my + sinf(a) * 0.075f);
    }
    glEnd();

    // Moon disc (warm white)
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(238, 243, 255, (unsigned char)(250 * alpha)); // background white circle
    glVertex2f(mx, my);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glColor4ub(228, 234, 255, (unsigned char)(240 * alpha));
        glVertex2f(mx + cosf(a) * 0.032f / asp, my + sinf(a) * 0.032f);
    }
    glEnd();

    // Crescent shadow — dark disc offset to the right
    // Its colour must exactly match the night sky at that position
    glBegin(GL_TRIANGLE_FAN);
    glColor4ub(5, 5, 20, (unsigned char)(255 * alpha)); // background cascked dark grey circle
    glVertex2f(mx + 0.013f / asp, my);
    for (int i = 0; i <= SEG; i++)
    {
        float a = i * 2.f * PI / SEG;
        glVertex2f(mx + 0.013f / asp + cosf(a) * 0.026f / asp,
                   my + sinf(a) * 0.026f);
    }
    glEnd();

    // Subtle surface detail — two faint craters
    glColor4ub(200, 210, 255, (unsigned char)(28 * alpha));
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(mx - 0.006f / asp, my + 0.010f);
    for (int i = 0; i <= 14; i++)
    {
        float a = i * 2.f * PI / 14.f;
        glVertex2f(mx - 0.006f / asp + cosf(a) * 0.007f / asp,
                   my + 0.010f + sinf(a) * 0.006f);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(mx - 0.010f / asp, my - 0.006f);
    for (int i = 0; i <= 12; i++)
    {
        float a = i * 2.f * PI / 12.f;
        glVertex2f(mx - 0.010f / asp + cosf(a) * 0.005f / asp,
                   my - 0.006f + sinf(a) * 0.004f);
    }
    glEnd();
}

// ================================================================
//  [D] drawDayNightGround()  — replaces the static ground quad.
// ================================================================
static void drawDayNightGround()
{
    float db = getDayBlend();

    float gr = lerpF(0.15f, 0.32f, db); // was 0.08, now 0.18 (much lighter)
    float gg = lerpF(0.30f, 0.52f, db); // was 0.22, now 0.32 (lighter green)
    float gb = lerpF(0.07f, 0.06f, db); // was 0.05, now 0.08 (slightly more blue)

    glColor3f(gr, gg, gb);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, -0.15f); // horizon lowered from -0.25 to -0.36
    glVertex2f(-1.0f, -0.15f);
    glEnd();
}
// ================================================================
//  [F] updateDayNight() speed 0.001 → ~3× slower, more cinematic //[Prottoy]
// ================================================================
static void updateDayNight()
{
    const float speed = 0.003f; // radians per tick — tune freely

    if (isDay)
    {
        dayAngle += speed;
        if (dayAngle >= PI) // PI=180
        {
            dayAngle = 0.0f;
            isDay = 0;
            nightAngle = 0.04f; // small offset so moon doesn't start at horizon edge
        }
    }
    else
    {
        nightAngle += speed;
        if (nightAngle >= PI)
        {
            nightAngle = 0.0f;
            isDay = 1;
            dayAngle = 0.04f;
        }
    }
}
// ----------------------------------------------------------------
//  Keyboard callback – force day, night, spring or rainy mode instantly
// ----------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'd': // full day (noon)
        isDay = 1;
        dayAngle = PI / 2.0f; // sun at zenith (90°)
        break;

    case 'n': // full night (midnight)
        isDay = 0;
        nightAngle = PI / 2.0f; // moon high (90°)
        break;

    case 'm': // morning (sunrise - sun just rising in the EAST)
        isDay = 1;
        dayAngle = 0.05f; // small angle = sun rising (left side)
        break;

    case 'e': // evening (sunset - sun just setting in the WEST)
        isDay = 1;
        dayAngle = PI - 0.25f; // near PI = sun setting (right side)
        break;

    case 's': // Spring mode (falling petals)
        springMode = true;
        rainMode = false;
        autumnMode = false;
        winterMode = false;
        summerMode = false;
        resetThunderstorm(); // kill lightning + sky flash
        stopRainSound();
        break;

    case 'r': // Rainy season mode (rain drops, umbrellas)
        rainMode = true;
        springMode = false;
        autumnMode = false;
        winterMode = false;
        summerMode = false;
        resetThunderstorm(); // plain rain — no thunder unless 't' pressed
        if (!rainSoundPlaying)
            playRainSound();
        break;

    case 'a': // Autumn mode — falling leaves (Emad)
        autumnMode = true;
        springMode = false;
        rainMode = false;
        winterMode = false;
        summerMode = false;
        resetThunderstorm();
        stopRainSound();
        break;

    case 'w': // Winter mode — falling snow (Shajmin)
        winterMode = true;
        springMode = false;
        rainMode = false;
        autumnMode = false;
        summerMode = false;
        resetThunderstorm();
        stopRainSound();
        break;

    case 'g': // Summer mode — mangoes, jackfruits, heat haze, stall, birds
        summerMode = true;
        springMode = false;
        rainMode = false;
        autumnMode = false;
        winterMode = false;
        resetThunderstorm();
        stopRainSound();
        break;
    case '+': // increase breeze speed
    case '=': // same physical key without Shift
        breezeSpeed += 0.004f;
        if (breezeSpeed > 0.080f)
            breezeSpeed = 0.080f; // cap at ~5× default
        break;

    case '-': // decrease breeze speed
    case '_':
        breezeSpeed -= 0.004f;
        if (breezeSpeed < 0.002f)
            breezeSpeed = 0.002f; // floor — never fully stops
        break;
    case 'z': // increase cloud size
        cloudScale += 0.1f;
        if (cloudScale > 3.0f)
            cloudScale = 3.0f; // cap at 3x default
        break;

    case 'x': // decrease cloud size
        cloudScale -= 0.1f;
        if (cloudScale < 0.2f)
            cloudScale = 0.2f; // floor at 0.2x default
        break;
    case '[': // decrease cloud speed
        cloudSpeedMult -= 0.2f;
        if (cloudSpeedMult < 0.0f)
            cloudSpeedMult = 0.0f; // can fully stop
        break;

    case ']': // increase cloud speed
        cloudSpeedMult += 0.2f;
        if (cloudSpeedMult > 5.0f)
            cloudSpeedMult = 5.0f; // cap at 5x
        break;

    // ── Dynamic translation: car speed control (Emad) ──
    // ',' slows the cars down (can fully stop them at 0.0).
    // '.' speeds them up.  Multiplier is applied inside updateTraffic().
    case ',':
    case '<':
        carSpeedMult -= 0.2f;
        if (carSpeedMult < 0.0f)
            carSpeedMult = 0.0f; // can fully stop
        break;

    case '.':
    case '>':
        carSpeedMult += 0.2f;
        if (carSpeedMult > 5.0f)
            carSpeedMult = 5.0f; // cap at 5x default
        break;

    // ── Dynamic scaling: autumn leaf size (Emad) ──
    // 'k' shrinks every falling leaf, 'l' grows them.  This is read
    // inside drawAutumn() and passed straight to glScalef().
    case 'k':
        leafScale -= 0.2f;
        if (leafScale < 0.2f)
            leafScale = 0.2f; // floor — never disappear
        break;

    case 'l':
        leafScale += 0.2f;
        if (leafScale > 4.0f)
            leafScale = 4.0f; // cap at 4x default
        break;

    // Dynamic translation: sports player speed [Shajmin]
    // 'h' slows every basketball + football player down.
    // 'j' speeds them up.  Multiplier is applied inside updatePlayers().
    case 'h': // [Shajmin]
        playerSpeedMult -= 0.2f;
        if (playerSpeedMult < 0.2f)
            playerSpeedMult = 0.2f; // floor — never freeze
        break;

    case 'j': // [Shajmin]
        playerSpeedMult += 0.2f;
        if (playerSpeedMult > 4.0f)
            playerSpeedMult = 4.0f; // cap at 4x default
        break;

    // Dynamic scaling: winter snowflake size [Shajmin]
    // 'v' shrinks every snowflake, 'b' grows them.  Read inside
    // drawWinter() and passed straight to glScalef().
    case 'v': // [Shajmin]
        snowScale -= 0.2f;
        if (snowScale < 0.4f)
            snowScale = 0.4f; // floor — keep flakes visible
        break;

    case 'b': // [Shajmin]
        snowScale += 0.2f;
        if (snowScale > 3.0f)
            snowScale = 3.0f; // cap at 3x default
        break;

    // Dynamic rotation: parking lot entry arm [Shajmin]
    // 'u' tells the arm to open  (target ≈ 85°, points up).
    // 'i' tells the arm to close (target =  0°, points right).
    // The arm eases towards parkingArmTarget every tick in update(),
    // so the rotation is smooth and clearly visible on screen.
    case 'u': // [Shajmin]
        parkingArmTarget = 85.0f;
        break;

    case 'i': // [Shajmin]
        parkingArmTarget = 0.0f;
        break;

    // ── Rain speed control (Sadia) ──
    case 'p': // speed up rain
        rainSpeedMult += 0.5f;
        if (rainSpeedMult > 5.0f)
            rainSpeedMult = 5.0f;
        break;

    case 'o': // slow down rain
        rainSpeedMult -= 0.5f;
        if (rainSpeedMult < 0.0f)
            rainSpeedMult = 0.0f;
        break;

    // ── Thunderstorm cycle (Sadia) ──
    // 1st press: intensity 1 (small).
    // 2nd press: intensity 2 (bigger + 1 branch).
    // 3rd press: intensity 3 (biggest + 3 branches).
    // 4th press: turn it off again.
    case 't':
        if (!thunderMode)
        {
            thunderMode = true;
            rainMode = true;
            springMode = false;
            autumnMode = false;
            winterMode = false;
            summerMode = false;
            thunderIntensity = 1;
            thunderTimer = 0.4f; // first bolt comes quickly
            lightningVisible = false;
            thunderFlashAlpha = 0.0f;
            if (!rainSoundPlaying)
                playRainSound();
        }
        else
        {
            thunderIntensity++;
            if (thunderIntensity > 3)
            {
                // 4th press — turn off
                thunderMode = false;
                thunderIntensity = 1;
                lightningVisible = false;
                thunderFlashAlpha = 0.0f;
            }
            else
            {
                thunderTimer = 0.3f; // show new size quickly
            }
        }
        break;

    // ── FIREWORKS KEY ────────────────────────────────────────────
    case 'f': // start fireworks show
        if (!fwActive)
        {
            fwActive = true;
            fwTimer = 0.0f;
            fwLaunchIdx = 0;
            // Kill any leftover particles from last show
            for (int i = 0; i < FW_MAX_PARTICLES; i++)
                fwParticles[i].active = false;
            for (int i = 0; i < FW_MAX_ROCKETS; i++)
                fwRockets[i].active = false;
            fwPlaySound();
        }
        break;

    default:
        return; // ignore other keys
    }
    glutPostRedisplay(); // redraw immediately
}
// ================================================================
//  PERFECTED drawSkyGradient() – Lerp style from sample + smooth twilight
static void drawSkyGradient()
{
    float angle = isDay ? dayAngle : (PI + nightAngle); // continuous arc
    float sunHeight = sinf(angle);

    // Normalized t: 0.0 = full day, 1.0 = full night
    float t = 1.0f - sunHeight;
    t = fmaxf(0.0f, fminf(1.0f, t)); // clamp like sample

    // ==================== COLOUR DEFINITIONS ====================
    // Day: bright blue
    const float dayTopR = 0.12f, dayTopG = 0.48f, dayTopB = 0.88f;
    const float dayHorR = 0.55f, dayHorG = 0.78f, dayHorB = 0.98f;

    // Twilight / Sunset: warm reddish-orange
    const float twiTopR = 0.98f, twiTopG = 0.45f, twiTopB = 0.25f;
    const float twiHorR = 1.00f, twiHorG = 0.62f, twiHorB = 0.38f;

    // Night: deep dark blue-purple
    const float nightTopR = 0.018f, nightTopG = 0.018f, nightTopB = 0.065f;
    const float nightHorR = 0.045f, nightHorG = 0.035f, nightHorB = 0.095f;

    // ==================== LERP BLEND ====================
    float topR = lerpF(dayTopR, twiTopR, t);
    float topG = lerpF(dayTopG, twiTopG, t);
    float topB = lerpF(dayTopB, twiTopB, t);

    float horR = lerpF(dayHorR, twiHorR, t);
    float horG = lerpF(dayHorG, twiHorG, t);
    float horB = lerpF(dayHorB, twiHorB, t);

    // Night override when fully dark
    if (t > 0.75f)
    {
        float nightT = (t - 0.75f) / 0.25f; // 0 → 1 in final quarter
        topR = lerpF(topR, nightTopR, nightT);
        topG = lerpF(topG, nightTopG, nightT);
        topB = lerpF(topB, nightTopB, nightT);

        horR = lerpF(horR, nightHorR, nightT);
        horG = lerpF(horG, nightHorG, nightT);
        horB = lerpF(horB, nightHorB, nightT);
    }

    // ==================== DRAW GRADIENT ====================
    glShadeModel(GL_SMOOTH);
    glBegin(GL_QUADS);
    glColor3f(topR, topG, topB);
    glVertex2f(-1.0f, 1.0f);
    glColor3f(topR, topG, topB);
    glVertex2f(1.0f, 1.0f);
    glColor3f(horR, horG, horB);
    glVertex2f(1.0f, -1.0f);
    glColor3f(horR, horG, horB);
    glVertex2f(-1.0f, -1.0f);
    glEnd();

    // Strong horizon glow during twilight (when t is around 0.4 ~ 0.7)
    if (t > 0.35f && t < 0.75f)
    {
        float glowA = (1.0f - fabsf(t - 0.55f) * 3.0f) * 0.65f;
        if (glowA > 0.0f)
        {
            glColor4f(1.00f, 0.55f, 0.25f, glowA);
            glBegin(GL_QUADS);
            glVertex2f(-1.0f, -0.22f);
            glVertex2f(1.0f, -0.22f);
            glVertex2f(1.0f, -0.06f);
            glVertex2f(-1.0f, -0.06f);
            glEnd();
        }
    }
}
// ================================================================
// BUILDING LIGHTS — PERFECTLY ALIGNED TO WINDOWS (Night Only)
// ================================================================
static void drawBuildingLights()
{
    float db = getDayBlend();
    float night = smoothStep(1.f - db);
    if (night < 0.02f)
        return;

    float winAlpha = night * night * 215.f; // main window light
    float glowAlpha = night * night * 75.f; // soft outer glow (unused but fine)

    // ============================================================
    // 1. D-BUILDING — Perfect window alignment
    // ============================================================
    {
        const float bx = -0.15f, by = -0.36f;
        const float bw = 0.90f, bh = 0.88f;
        const int NF = 10;
        const float fH = bh / (float)NF;
        const float glassRight = bx + bw * 0.265f;

        // Regular floors (non-lounge, non-top)
        for (int f = 0; f < NF; f++)
        {
            if (f == 3 || f == 6 || f == NF - 1)
                continue;

            float fy = by + f * fH;
            float wX = glassRight + 0.014f;
            float wW = bx + bw - 0.010f - wX;
            float wY = fy + fH * 0.14f;
            float wH = fH * 0.67f;

            int nDiv = 26;
            float paneW = wW / (float)nDiv;

            for (int d = 0; d < nDiv; d++)
            {
                bool lit = ((f * 17 + d * 7 + f * d) % 10) < 7;
                if (!lit)
                    continue;

                float px1 = wX + d * paneW + 0.0015f;
                float px2 = px1 + paneW - 0.003f;

                glColor4ub(255, 228, 135, (unsigned char)winAlpha);
                glBegin(GL_QUADS);
                glVertex2f(px1, wY + 0.003f);
                glVertex2f(px2, wY + 0.003f);
                glVertex2f(px2, wY + wH - 0.003f);
                glVertex2f(px1, wY + wH - 0.003f);
                glEnd();
            }
        }

        // Lounge floors (4th & 7th)
        int loungeF[] = {3, 6};
        for (int i = 0; i < 2; i++)
        {
            int f = loungeF[i];
            float fy = by + f * fH;
            float lX = glassRight + 0.035f;
            float lW = bw * 0.64f;
            float lY = fy + fH * 0.25f;
            float lH = fH * 0.26f;
            glColor4ub(170, 235, 145, (unsigned char)(winAlpha * 0.75f));
            glBegin(GL_QUADS);
            glVertex2f(lX, lY);
            glVertex2f(lX + lW, lY);
            glVertex2f(lX + lW, lY + lH);
            glVertex2f(lX, lY + lH);
            glEnd();
        }

        // ---- ADDED: Soft warm glow on left curtain glass (less bright) ----
        glColor4ub(255, 220, 160, (unsigned char)(night * night * 25.f));
        glBegin(GL_POLYGON);
        glVertex2f(bx, by);
        glVertex2f(glassRight, by);
        glVertex2f(glassRight, by + bh * 0.85f);
        glVertex2f(bx + bw * 0.03f, by + bh);
        glVertex2f(bx, by + bh);
        glEnd();

        // Ground lobby
        {
            float lobX = bx + bw * 0.30f, lobW = bw * 0.38f;
            float lobH = fH * 0.915f;
            glColor4ub(255, 235, 160, (unsigned char)(night * 185.f));
            glBegin(GL_QUADS);
            glVertex2f(lobX, by);
            glVertex2f(lobX + lobW, by);
            glVertex2f(lobX + lobW, by + lobH);
            glVertex2f(lobX, by + lobH);
            glEnd();
        }
    }

    // ============================================================
    // 2. C-BUILDING — Sphere-boundary-conforming band lights
    // ============================================================
    {
        const float cx = -0.715f;
        const float podBot = -0.360f;
        const float podH = 0.168f;
        const float podTop = podBot + podH; // -0.192
        const float srx = 0.190f, sry = 0.250f;
        const float cy = podTop + sry * 0.78f; // matches drawCBuilding

        // Reconstruct the same 14 band boundaries as drawCBuilding
        float fY[14];
        float lowerHeight = 2.0f * sry - 0.28f * sry;
        fY[0] = cy - sry;
        for (int i = 1; i <= 12; i++)
            fY[i] = fY[0] + (i / 12.0f) * lowerHeight;
        fY[13] = cy + sry;

        const int nPts = 80; // arc resolution — higher = smoother boundary

        for (int band = 2; band <= 12; band += 2)
        {
            bool lit = ((band / 2) % 3) != 0;
            if (!lit)
                continue;

            float yBot = fY[band - 1] + 0.003f; // inner margin
            float yTop = fY[band] - 0.003f;

            // Clamp to sphere height
            yBot = fmaxf(yBot, cy - sry + 0.001f);
            yTop = fminf(yTop, cy + sry - 0.001f);
            if (yTop <= yBot)
                continue;

            glColor4ub(255, 255, 190, (unsigned char)(winAlpha * 0.98f));
            glBegin(GL_POLYGON);

            // Bottom edge: left → right  (uses actual sphere half-width at yBot)
            for (int p = 0; p <= nPts; p++)
            {
                float t = (float)p / nPts;
                float yy = yBot;
                float ratio = (yy - cy) / sry;
                float hw = srx * sqrtf(fmaxf(0.f, 1.f - ratio * ratio)) * 0.97f;
                glVertex2f(cx - hw + t * 2.f * hw, yy);
            }

            // Top edge: right → left  (closes the polygon against the sphere)
            for (int p = nPts; p >= 0; p--)
            {
                float t = (float)p / nPts;
                float yy = yTop;
                float ratio = (yy - cy) / sry;
                float hw = srx * sqrtf(fmaxf(0.f, 1.f - ratio * ratio)) * 0.97f;
                glVertex2f(cx - hw + t * 2.f * hw, yy);
            }

            glEnd();
        }
    }

    // ============================================================
    // LAMP POST NIGHT HALO BOOST
    // ============================================================
    {
        float lampPosX[] = {-0.83f, -0.18f, 0.41f, -0.67f, -0.13f, 0.59f};
        float lampPosY[] = {-0.708f, -0.708f, -0.708f, -0.990f, -0.990f, -0.990f};
        const float postH = 0.195f;

        float night2 = smoothStep(1.f - getDayBlend());
        if (night2 < 0.05f)
            night2 = 0.0f;

        for (int l = 0; l < 6; l++)
        {
            float lx = lampPosX[l];
            float ly = lampPosY[l];
            float armX = lx + 0.046f;
            float armY = ly + postH * 0.86f + 0.028f;

            glBegin(GL_TRIANGLE_FAN);
            glColor4ub(255, 235, 130, (unsigned char)(night2 * 68.f));
            glVertex2f(armX, armY - 0.004f);
            for (int k = 0; k <= 28; k++)
            {
                float a = k * 2.f * PI / 28.f;
                glColor4ub(255, 220, 95, 0);
                glVertex2f(armX + cosf(a) * 0.118f, armY + sinf(a) * 0.092f);
            }
            glEnd();

            float poolCX = armX;
            float poolCY = ly + 0.006f;
            glBegin(GL_TRIANGLE_FAN);
            glColor4ub(255, 225, 110, (unsigned char)(night2 * 42.f));
            glVertex2f(poolCX, poolCY);
            for (int k = 0; k <= 24; k++)
            {
                float a = k * 2.f * PI / 24.f;
                glColor4ub(255, 205, 75, 0);
                glVertex2f(poolCX + cosf(a) * 0.105f, poolCY + sinf(a) * 0.029f);
            }
            glEnd();

            glBegin(GL_TRIANGLE_FAN);
            glColor4ub(255, 245, 160, (unsigned char)(night2 * 28.f));
            glVertex2f(poolCX, poolCY);
            for (int k = 0; k <= 18; k++)
            {
                float a = k * 2.f * PI / 18.f;
                glColor4ub(255, 230, 120, 0);
                glVertex2f(poolCX + cosf(a) * 0.058f, poolCY + sinf(a) * 0.016f);
            }
            glEnd();
        }
    }
}
static void drawLeftBackgroundLights()
{
    float db = getDayBlend();
    float night = smoothStep(1.f - db);
    if (night < 0.02f)
        return;
    float winAlpha = night * night * 220.f;
    float darkAlpha = night * night * 75.f; // dim alpha for unlit windows

    glPushMatrix();
    glTranslatef(-0.733f, 0.098f, 0.0f);
    glScalef(0.282f, 0.48f, 1.0f);

    struct BGTower
    {
        float x1, y1, x2, y2;
        int c, r;
    };
    BGTower towers[] = {
        {0.44f, -0.83f, 0.75f, 0.28f, 2, 5},
        {0.15f, -0.83f, 0.52f, 0.50f, 3, 7},
        {-0.16f, -0.83f, 0.22f, 0.42f, 3, 6},
        {-0.52f, -0.83f, -0.08f, 0.68f, 3, 9},
        {-0.94f, -0.83f, -0.44f, 0.95f, 4, 12},
    };

    for (int t = 0; t < 5; t++)
    {
        BGTower &tw = towers[t];
        float bw2 = tw.x2 - tw.x1;
        float bh2 = tw.y2 - tw.y1;
        float wW = (bw2 * 0.55f) / tw.c;
        float wH = (bh2 * 0.50f) / tw.r;
        float xGap = (bw2 - tw.c * wW) / (tw.c + 1.f);
        float yGap = (bh2 - tw.r * wH) / (tw.r + 1.f);

        for (int row = 0; row < tw.r; row++)
        {
            for (int col = 0; col < tw.c; col++)
            {
                bool lit = ((row * 3 + col * 7 + t * 11) % 10) < 7;
                float wx = tw.x1 + xGap + col * (wW + xGap);
                float wy = tw.y1 + yGap + row * (wH + yGap);

                if (lit)
                {
                    // Bright warm yellow window
                    glColor4ub(255, 220, 115, (unsigned char)winAlpha);
                }
                else
                {
                    // Dark bluish-grey — office is off
                    glColor4ub(18, 28, 52, (unsigned char)darkAlpha);
                }
                glBegin(GL_QUADS);
                glVertex2f(wx, wy);
                glVertex2f(wx + wW, wy);
                glVertex2f(wx + wW, wy + wH);
                glVertex2f(wx, wy + wH);
                glEnd();
            }
        }
    }
    glPopMatrix();
}
static void drawDCurtainLightUnder()
{
    float db = getDayBlend();
    float night = smoothStep(1.f - db);
    if (night < 0.02f)
        return;
    float nightFactor = night * night;

    const float bx = -0.15f, by = -0.36f;
    const float bw = 0.90f, bh = 0.88f;
    const int NF = 10;
    const float fH = bh / (float)NF;
    const float glassRight = bx + bw * 0.265f;
    const float leftMargin = 0.018f;
    const float rightMargin = 0.018f;

    // Soft base glow across entire curtain wall
    glColor4ub(245, 210, 150, (unsigned char)(nightFactor * 90.f));
    glBegin(GL_POLYGON);
    glVertex2f(bx, by);
    glVertex2f(glassRight, by);
    glVertex2f(glassRight, by + bh - 0.168f * bh);
    glVertex2f(bx + bw * 0.03f, by + bh);
    glVertex2f(bx, by + bh);
    glEnd();

    // Per-floor lit bands — drawn BEFORE mullions so lines render on top
    for (int f = 0; f < NF; f++)
    {
        if (f == 2 || f == 6 || f == 8)
            continue; // skip a few for variety
        float fy = by + f * fH;
        float wY = fy + fH * 0.16f;
        float wH = fH * 0.66f;
        // Vary brightness per floor deterministically
        float brightness = 0.75f + 0.25f * ((f * 7) % 5) / 4.0f;
        unsigned char alpha = (unsigned char)(nightFactor * 180.f * brightness);
        glColor4ub(255, 220, 160, alpha);
        glBegin(GL_QUADS);
        glVertex2f(bx + leftMargin, wY);
        glVertex2f(glassRight - rightMargin, wY);
        glVertex2f(glassRight - rightMargin, wY + wH);
        glVertex2f(bx + leftMargin, wY + wH);
        glEnd();
    }
}
static void drawAnnex9Lights()
{
    float db = getDayBlend();
    float night = smoothStep(1.f - db);
    if (night < 0.02f)
        return;

    float winAlpha = night * night * 219.f;
    float darkAlpha = night * night * 75.f;

    glPushMatrix();
    glTranslatef(-0.445f, -0.2085f, 0.0f);
    glScalef(0.35235f, 0.36066f, 1.0f);

    // Central blue tower floor bands
    float divY_l[] = {-0.42f, -0.23f, -0.06f, 0.11f, 0.28f, 0.45f, 0.62f, 0.80f};
    for (int i = 0; i < 7; i++)
    {
        bool lit = (i == 1 || i == 4 || i == 5);
        float y1 = divY_l[i] + 0.018f;
        float y2 = divY_l[i + 1] - 0.006f;
        if (lit)
            glColor4ub(255, 255, 180, (unsigned char)(winAlpha * 0.88f));
        else
            glColor4ub(12, 20, 48, (unsigned char)darkAlpha);
        glBegin(GL_QUADS);
        glVertex2f(-0.28f, y1);
        glVertex2f(0.28f, y1);
        glVertex2f(0.28f, y2);
        glVertex2f(-0.28f, y2);
        glEnd();
    }

    // Side wing windows
    const float winW = 0.18f;
    const float winH = 0.12f;
    const float pad = 0.014f;
    const float lcx[] = {-0.79f, -0.54f};
    const float rcx[] = {0.36f, 0.61f};
    const float rowY[] = {-0.32f, -0.09f, 0.13f, 0.35f, 0.57f};

    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 2; col++)
        {
            bool litL = ((row * 5 + col * 9 + 3) % 10) < 7;
            bool litR = ((row * 5 + col * 9 + 7) % 10) < 7;

            // Left wing
            glColor4ub(255, 218, 115, litL ? (unsigned char)winAlpha : (unsigned char)darkAlpha);
            glBegin(GL_QUADS);
            glVertex2f(lcx[col] + pad, rowY[row] + pad);
            glVertex2f(lcx[col] + winW - pad, rowY[row] + pad);
            glVertex2f(lcx[col] + winW - pad, rowY[row] + winH - pad);
            glVertex2f(lcx[col] + pad, rowY[row] + winH - pad);
            glEnd();

            // Right wing
            glColor4ub(255, 218, 115, litR ? (unsigned char)winAlpha : (unsigned char)darkAlpha);
            glBegin(GL_QUADS);
            glVertex2f(rcx[col] + pad, rowY[row] + pad);
            glVertex2f(rcx[col] + winW - pad, rowY[row] + pad);
            glVertex2f(rcx[col] + winW - pad, rowY[row] + winH - pad);
            glVertex2f(rcx[col] + pad, rowY[row] + winH - pad);
            glEnd();
        }
    }

    glPopMatrix();
}
// ================================================================
//  SUMMER SEASON EXTRA DECORATIONS (Stall, kids, birds, dry patches, extra heat haze)
// ================================================================
static void drawCoconutStall()
{
    float baseX = -0.28f, baseY = -0.715f;
    solidQ(baseX - 0.06f, baseY, baseX + 0.06f, baseY + 0.025f, 120, 85, 45);
    solidQ(baseX - 0.07f, baseY + 0.022f, baseX + 0.07f, baseY + 0.032f, 185, 140, 70);
    glColor3ub(90, 70, 30);
    diskFan(baseX - 0.025f, baseY + 0.028f, 0.012f, 0.010f, 12, 90, 70, 30);
    diskFan(baseX + 0.025f, baseY + 0.030f, 0.012f, 0.010f, 12, 90, 70, 30);
    glColor3ub(60, 120, 30);
    diskFan(baseX - 0.025f, baseY + 0.032f, 0.005f, 0.004f, 8, 60, 120, 30);
    diskFan(baseX + 0.025f, baseY + 0.034f, 0.005f, 0.004f, 8, 60, 120, 30);
    glColor3ub(200, 40, 40);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(baseX, baseY + 0.10f);
    for (int k = 0; k <= 12; k++)
    {
        float a = k * 2.f * PI / 12.f;
        glVertex2f(baseX + cosf(a) * 0.09f, baseY + 0.10f + sinf(a) * 0.07f);
    }
    glEnd();
    glColor3ub(240, 240, 220);
    for (int k = 0; k < 8; k++)
    {
        float a = k * 2.f * PI / 8.f;
        float x1 = baseX + cosf(a) * 0.09f;
        float y1 = baseY + 0.10f + sinf(a) * 0.07f;
        float x2 = baseX + cosf(a + 0.4f) * 0.07f;
        float y2 = baseY + 0.10f + sinf(a + 0.4f) * 0.05f;
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }
    solidQ(baseX - 0.008f, baseY + 0.03f, baseX + 0.008f, baseY + 0.10f, 95, 70, 40);
}

static void drawKidsEatingMango()
{
    float baseX = 0.22f, baseY = -0.715f;
    diskFan(baseX - 0.032f, baseY + 0.030f, 0.008f, 0.009f, 10, 210, 150, 70);
    solidQ(baseX - 0.045f, baseY + 0.010f, baseX - 0.018f, baseY + 0.028f, 50, 150, 210);
    solidQ(baseX - 0.042f, baseY - 0.008f, baseX - 0.020f, baseY + 0.010f, 25, 35, 55);
    diskFan(baseX - 0.018f, baseY + 0.022f, 0.006f, 0.008f, 10, 240, 160, 40);
    diskFan(baseX + 0.032f, baseY + 0.028f, 0.008f, 0.009f, 10, 210, 150, 70);
    solidQ(baseX + 0.020f, baseY + 0.008f, baseX + 0.046f, baseY + 0.026f, 220, 80, 80);
    solidQ(baseX + 0.022f, baseY - 0.008f, baseX + 0.044f, baseY + 0.008f, 25, 35, 55);
    diskFan(baseX + 0.048f, baseY + 0.020f, 0.006f, 0.008f, 10, 240, 160, 40);
}

static void drawBirdsResting()
{
    float lampX[] = {-0.83f, -0.18f, 0.41f, -0.67f, -0.13f, 0.59f};
    float lampY[] = {-0.708f, -0.708f, -0.708f, -0.990f, -0.990f, -0.990f};
    float postH = 0.195f;
    for (int i = 0; i < 6; i += 2)
    {
        float armY = lampY[i] + postH * 0.86f + 0.028f;
        float armX = lampX[i] + 0.046f;
        glColor3ub(40, 40, 45);
        glBegin(GL_TRIANGLES);
        glVertex2f(armX - 0.008f, armY + 0.015f);
        glVertex2f(armX, armY + 0.010f);
        glVertex2f(armX + 0.008f, armY + 0.015f);
        glEnd();
        diskFan(armX, armY + 0.018f, 0.003f, 0.003f, 6, 40, 40, 45);
    }
}

static void drawDryGroundPatches()
{
    glColor4ub(140, 100, 40, 180);
    glBegin(GL_POLYGON);
    glVertex2f(0.50f, -0.690f);
    glVertex2f(0.55f, -0.695f);
    glVertex2f(0.58f, -0.685f);
    glVertex2f(0.54f, -0.670f);
    glVertex2f(0.48f, -0.678f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(-0.75f, -0.685f);
    glVertex2f(-0.70f, -0.690f);
    glVertex2f(-0.66f, -0.680f);
    glVertex2f(-0.70f, -0.665f);
    glVertex2f(-0.76f, -0.672f);
    glEnd();
    glColor3ub(90, 60, 20);
    glLineWidth(0.8f);
    glBegin(GL_LINES);
    glVertex2f(0.52f, -0.688f);
    glVertex2f(0.56f, -0.680f);
    glVertex2f(0.50f, -0.672f);
    glVertex2f(0.55f, -0.678f);
    glVertex2f(-0.72f, -0.682f);
    glVertex2f(-0.68f, -0.672f);
    glVertex2f(-0.74f, -0.668f);
    glVertex2f(-0.68f, -0.684f);
    glEnd();
}

static void drawWavyHeatHaze()
{
    float db = getDayBlend();
    if (!summerMode || db < 0.6f)
        return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    int nWaves = 20;
    float alpha = 0.05f;
    glColor4f(1.0f, 0.95f, 0.7f, alpha);
    glLineWidth(0.4f);
    for (int w = 0; w < nWaves; w++)
    {
        float y = -0.6f + w * 0.05f;
        glBegin(GL_LINE_STRIP);
        for (float x = -1.0f; x <= 1.0f; x += 0.05f)
        {
            float offset = sinf(x * 20.0f + w * 0.5f) * 0.005f;
            glVertex2f(x, y + offset);
        }
        glEnd();
    }
    glColor4f(1.0f, 0.85f, 0.5f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.25f);
    glVertex2f(1.0f, -0.25f);
    glVertex2f(1.0f, -0.10f);
    glVertex2f(-1.0f, -0.10f);
    glEnd();
}
// ================================================================
//  SUMMER SEASON CUSTOM SKY / GROUND /
// ================================================================
static void drawSummerSky()
{
    float db = getDayBlend();
    float night = 1.0f - db;
    float topR = lerpF(60.0f, 8.0f, night);
    float topG = lerpF(140.0f, 12.0f, night);
    float topB = lerpF(210.0f, 32.0f, night);
    float botR = lerpF(255.0f, 25.0f, night);
    float botG = lerpF(200.0f, 20.0f, night);
    float botB = lerpF(120.0f, 40.0f, night);
    glBegin(GL_QUADS);
    glColor3f(topR / 255.0f, topG / 255.0f, topB / 255.0f);
    glVertex2f(-1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glColor3f(botR / 255.0f, botG / 255.0f, botB / 255.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

static void drawSummerGround()
{
    float db = getDayBlend();
    float night = 1.0f - db;
    float gr = lerpF(0.55f, 0.08f, night);
    float gg = lerpF(0.45f, 0.10f, night);
    float gb = lerpF(0.25f, 0.05f, night);
    glColor3f(gr, gg, gb);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, -0.15f);
    glVertex2f(-1.0f, -0.15f);
    glEnd();
}

static void drawSummerTint()
{
    float db = getDayBlend();
    float strength = db * 0.15f;
    glColor4f(1.0f, 0.85f, 0.45f, strength);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
}

static void drawSummerHeatHaze()
{
    float db = getDayBlend();
    if (db < 0.2f)
        return;
    unsigned char hazeA = (unsigned char)(db * 45.0f);
    glColor4ub(255, 220, 140, hazeA);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.72f);
    glVertex2f(1.0f, -0.72f);
    glVertex2f(1.0f, -0.65f);
    glVertex2f(-1.0f, -0.65f);
    glEnd();
    glColor4ub(255, 235, 170, (unsigned char)(hazeA * 0.6f));
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.65f);
    glVertex2f(1.0f, -0.65f);
    glVertex2f(1.0f, -0.55f);
    glVertex2f(-1.0f, -0.55f);
    glEnd();
}

// ================================================================
//  CLOUD SYSTEM FOR SPRING SEASON
// ================================================================

// ----------------------------------------------------------------
//  drawSingleCloud  — draws one fluffy cloud at (cx, cy) with given size
// ----------------------------------------------------------------
static void drawSingleCloud(float cx, float cy, float size)
{
    float s = size * cloudScale; // apply global scale to this cloud's size

    glColor4ub(245, 248, 250, 220);

    diskFan(cx, cy, s * 0.45f, s * 0.35f, 20, 245, 248, 250);
    diskFan(cx - s * 0.35f, cy - s * 0.08f, s * 0.38f, s * 0.30f, 20, 248, 250, 252);
    diskFan(cx + s * 0.35f, cy - s * 0.08f, s * 0.38f, s * 0.30f, 20, 248, 250, 252);
    diskFan(cx - s * 0.18f, cy + s * 0.12f, s * 0.32f, s * 0.28f, 20, 250, 252, 255);
    diskFan(cx + s * 0.18f, cy + s * 0.12f, s * 0.32f, s * 0.28f, 20, 250, 252, 255);
    diskFan(cx, cy + s * 0.18f, s * 0.35f, s * 0.28f, 20, 252, 254, 255);

    glColor4ub(220, 225, 230, 180);
    diskFan(cx - s * 0.25f, cy - s * 0.12f, s * 0.30f, s * 0.22f, 16, 220, 225, 230);
    diskFan(cx + s * 0.25f, cy - s * 0.12f, s * 0.30f, s * 0.22f, 16, 220, 225, 230);
    diskFan(cx, cy - s * 0.15f, s * 0.32f, s * 0.24f, 16, 218, 223, 228);
}

// ----------------------------------------------------------------
//  drawSpringClouds  — draws all clouds with day/night fading
// ----------------------------------------------------------------
static void drawSpringClouds()
{
    // Only show clouds during spring mode
    if (!springMode || rainMode || autumnMode || winterMode || summerMode)
        return;

    // Fade clouds based on daylight (more visible during day, fade at night)
    float db = getDayBlend();
    float cloudAlpha = db * 0.85f; // clouds visible during day, fade at night

    if (cloudAlpha < 0.05f)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        if (cloudActive[i])
        {
            // Adjust alpha based on cloud position (slightly transparent)
            glColor4ub(245, 248, 250, (unsigned char)(220 * cloudAlpha));
            drawSingleCloud(cloudX[i], cloudY[i], cloudSize[i]);
        }
    }
}

// ----------------------------------------------------------------
//  initSpringClouds  — initialize cloud positions and properties
// ----------------------------------------------------------------
static void initSpringClouds()
{
    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        // Random positions across the sky
        cloudX[i] = -1.2f + (rand() % 240) / 100.0f;
        cloudY[i] = 0.35f + (rand() % 50) / 100.0f;
        cloudSize[i] = 0.12f + (rand() % 80) / 500.0f;
        cloudSpeed[i] = 0.0008f + (rand() % 10) / 5000.0f;
        cloudActive[i] = true;
    }
}

// ----------------------------------------------------------------
//  updateSpringClouds  — slowly move clouds across the sky
// ----------------------------------------------------------------
static void updateSpringClouds()
{
    if (!springMode || rainMode || autumnMode || winterMode || summerMode)
        return;

    for (int i = 0; i < MAX_CLOUDS; i++)
    {
        if (cloudActive[i])
        {
            // Move clouds slowly to the right
            cloudX[i] += cloudSpeed[i] * cloudSpeedMult;

            // Reset cloud when it goes off-screen
            if (cloudX[i] > 1.5f)
            {
                cloudX[i] = -1.5f;
                cloudY[i] = 0.35f + (rand() % 50) / 100.0f;
                cloudSize[i] = 0.12f + (rand() % 80) / 500.0f;
                cloudSpeed[i] = 0.0008f + (rand() % 10) / 5000.0f;
            }
        }
    }
}

// ================================================================
//  THUNDERSTORM (Sadia) — jagged lightning bolts with scale grow,
//  optional branches, sky flash, and ground impact glow.
// ================================================================
static void generateLightningBolt(float startX, float startY,
                                  float endX, float endY,
                                  int intensity)
{
    // More segments = more jagged at higher intensity
    lgSegCount = 4 + intensity * 3; // lvl1=7, lvl2=10, lvl3=13
    if (lgSegCount > MAX_LIGHTNING_SEGS)
        lgSegCount = MAX_LIGHTNING_SEGS;

    lgSegX[0] = startX;
    lgSegY[0] = startY;

    float dx = (endX - startX) / lgSegCount;
    float dy = (endY - startY) / lgSegCount;

    // Jag amount grows with intensity
    float jagAmt = 0.04f + intensity * 0.055f; // lvl1=0.095, lvl2=0.15, lvl3=0.205

    for (int i = 1; i < lgSegCount; i++)
    {
        lgSegX[i] = startX + dx * i + fwRandSym() * jagAmt;
        lgSegY[i] = startY + dy * i + fwRandSym() * jagAmt * 0.4f;
    }
    lgSegX[lgSegCount] = endX;
    lgSegY[lgSegCount] = endY;

    // Generate branches at intensity 2+
    branchCount = 0;
    if (intensity >= 2)
    {
        int numBranches = (intensity == 2) ? 1 : MAX_BRANCHES;
        for (int b = 0; b < numBranches && b < MAX_BRANCHES; b++)
        {
            int splitSeg = 2 + (int)(fwRand() * (lgSegCount - 3));
            float bStartX = lgSegX[splitSeg];
            float bStartY = lgSegY[splitSeg];

            float bLen = 0.15f + intensity * 0.08f;
            float bAngle = fwRandSym() * 0.9f;
            float bEndX = bStartX + cosf(bAngle) * bLen;
            float bEndY = bStartY - sinf(fabsf(bAngle)) * bLen * 0.6f;

            int bSegs = 3 + intensity;
            branchX[b][0] = bStartX;
            branchY[b][0] = bStartY;

            float bdx = (bEndX - bStartX) / bSegs;
            float bdy = (bEndY - bStartY) / bSegs;
            float bJag = jagAmt * 0.55f;

            for (int i = 1; i < bSegs; i++)
            {
                branchX[b][i] = bStartX + bdx * i + fwRandSym() * bJag;
                branchY[b][i] = bStartY + bdy * i + fwRandSym() * bJag * 0.4f;
            }
            branchX[b][bSegs] = bEndX;
            branchY[b][bSegs] = bEndY;
            branchCount++;
        }
    }
}

static void drawThunderstorm()
{
    if (!thunderMode)
        return;

    // Sky flash
    if (thunderFlashAlpha > 0.01f)
    {
        glColor4f(0.88f, 0.93f, 1.0f, thunderFlashAlpha);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glEnd();
        thunderFlashAlpha *= 0.76f;
        if (thunderFlashAlpha < 0.01f)
            thunderFlashAlpha = 0.0f;
    }

    if (!lightningVisible || lgSegCount == 0)
        return;

    // Current scale factor (0 = invisible, 1 = full size)
    float sc = thunderScaling ? thunderScale / fmaxf(thunderScaleMax, 0.001f)
                              : 1.0f;
    if (sc <= 0.001f)
        return;

    float alpha = lightningTimer;

    float outerW = (4.0f + thunderIntensity * 4.0f) * sc;
    float midW = (2.0f + thunderIntensity * 2.0f) * sc;
    float coreW = (0.8f + thunderIntensity * 0.7f) * sc;

    // Push a scale matrix around the bolt's start point so it
    // appears to "shoot out" from the sky, growing toward the ground.
    float originX = lgSegX[0];
    float originY = lgSegY[0];

    glPushMatrix();
    glTranslatef(originX, originY, 0.0f);
    glScalef(sc, sc, 1.0f);
    glTranslatef(-originX, -originY, 0.0f);

    // ── MAIN BOLT ──
    glColor4f(0.5f, 0.65f, 1.0f, alpha * 0.35f);
    glLineWidth(outerW);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= lgSegCount; i++)
        glVertex2f(lgSegX[i], lgSegY[i]);
    glEnd();

    glColor4f(0.78f, 0.88f, 1.0f, alpha * 0.65f);
    glLineWidth(midW);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= lgSegCount; i++)
        glVertex2f(lgSegX[i], lgSegY[i]);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glLineWidth(coreW);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= lgSegCount; i++)
        glVertex2f(lgSegX[i], lgSegY[i]);
    glEnd();

    // ── BRANCHES ──
    for (int b = 0; b < branchCount; b++)
    {
        int bSegs = 3 + thunderIntensity;
        glColor4f(0.6f, 0.75f, 1.0f, alpha * 0.30f);
        glLineWidth(midW * 0.55f);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= bSegs; i++)
            glVertex2f(branchX[b][i], branchY[b][i]);
        glEnd();

        glColor4f(1.0f, 1.0f, 1.0f, alpha * 0.75f);
        glLineWidth(coreW * 0.55f);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= bSegs; i++)
            glVertex2f(branchX[b][i], branchY[b][i]);
        glEnd();
    }

    // ── GROUND IMPACT GLOW ──
    float gx = lgSegX[lgSegCount];
    float gy = lgSegY[lgSegCount];
    float gRx = (0.06f + thunderIntensity * 0.055f) * sc;
    float gRy = gRx * 0.40f;
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.85f, 0.92f, 1.0f, alpha * 0.70f * sc);
    glVertex2f(gx, gy);
    for (int k = 0; k <= 24; k++)
    {
        float a = k * 2.f * PI / 24.f;
        glColor4f(0.55f, 0.70f, 1.0f, 0.0f);
        glVertex2f(gx + cosf(a) * gRx, gy + sinf(a) * gRy);
    }
    glEnd();

    glPopMatrix();
    glLineWidth(1.0f);
}

static void updateThunderstorm(float dt)
{
    if (!thunderMode)
        return;

    // ── Scale animation update (grow then shrink) ──
    if (thunderScaling)
    {
        const float GROW_SPEED = 6.0f;
        const float SHRINK_SPEED = 3.5f;

        if (thunderScaleDir > 0.0f) // growing phase
        {
            thunderScale += GROW_SPEED * dt;
            if (thunderScale >= thunderScaleMax)
            {
                thunderScale = thunderScaleMax;
                thunderScaleDir = -1.0f;
            }
        }
        else // shrinking phase
        {
            thunderScale -= SHRINK_SPEED * dt;
            if (thunderScale <= 0.0f)
            {
                thunderScale = 0.0f;
                thunderScaling = false;
                lightningVisible = false;
            }
        }
        lightningTimer = thunderScale / fmaxf(thunderScaleMax, 0.001f);
    }

    // ── Interval countdown — fire a new bolt when timer expires ──
    thunderTimer -= dt;
    if (thunderTimer <= 0.0f)
    {
        float sx = fwRandSym() * 0.85f;
        float sy = 0.60f + fwRand() * 0.35f;
        float ex = sx + fwRandSym() * 0.12f;
        float ey = -0.65f + fwRand() * 0.35f;
        generateLightningBolt(sx, sy, ex, ey, thunderIntensity);

        lightningVisible = true;
        lightningTimer = 1.0f;

        thunderScaleMax = 1.0f;
        thunderScale = 0.0f;
        thunderScaleDir = 1.0f;
        thunderScaling = true;

        thunderFlashAlpha = 0.10f + thunderIntensity * 0.10f;
        thunderTimer = 2.0f + fwRand() * 2.5f;
    }

    if (lightningVisible && !thunderScaling)
    {
        lightningTimer -= dt * 3.5f;
        if (lightningTimer <= 0.0f)
        {
            lightningTimer = 0.0f;
            lightningVisible = false;
        }
    }
}

// ================================================================
//  DISPLAY
// ================================================================
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // ── RAINY SEASON (overrides normal sky) - SIMPLE SMOOTH TRANSITION ──
    if (rainMode)
    {
        float db = getDayBlend();      // 0 = night, 1 = day
        float nightFactor = 1.0f - db; // 0 = day, 1 = night

        // Use smoothStep for natural transition
        float smoothNight = smoothStep(nightFactor);

        // Sky top colors - interpolate between day and night
        float topR = lerpF(75.0f, 8.0f, smoothNight);
        float topG = lerpF(85.0f, 10.0f, smoothNight);
        float topB = lerpF(95.0f, 12.0f, smoothNight);

        // Sky bottom colors - interpolate between day and night
        float botR = lerpF(130.0f, 15.0f, smoothNight);
        float botG = lerpF(135.0f, 18.0f, smoothNight);
        float botB = lerpF(140.0f, 22.0f, smoothNight);

        // Draw rainy sky gradient
        glBegin(GL_QUADS);
        glColor4f(topR / 255.0f, topG / 255.0f, topB / 255.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glColor4f(topR / 255.0f, topG / 255.0f, topB / 255.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);
        glColor4f(botR / 255.0f, botG / 255.0f, botB / 255.0f, 1.0f);
        glVertex2f(1.0f, -1.0f);
        glColor4f(botR / 255.0f, botG / 255.0f, botB / 255.0f, 1.0f);
        glVertex2f(-1.0f, -1.0f);
        glEnd();

        // Mist colors - interpolate
        float mistR = lerpF(80.0f, 30.0f, smoothNight);
        float mistG = lerpF(85.0f, 32.0f, smoothNight);
        float mistB = lerpF(90.0f, 35.0f, smoothNight);

        // Mist alpha - denser at night
        float mistAlpha1 = lerpF(25.0f, 70.0f, smoothNight);
        float mistAlpha2 = lerpF(35.0f, 90.0f, smoothNight);

        glColor4f(mistR / 255.0f, mistG / 255.0f, mistB / 255.0f, mistAlpha1 / 255.0f);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, -0.15f);
        glVertex2f(-1.0f, -0.15f);
        glEnd();

        glColor4f(mistR / 255.0f, mistG / 255.0f, mistB / 255.0f, mistAlpha2 / 255.0f);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, -0.30f);
        glVertex2f(-1.0f, -0.30f);
        glEnd();

        // Ground colors - interpolate
        float groundR = lerpF(60.0f, 25.0f, smoothNight);
        float groundG = lerpF(85.0f, 35.0f, smoothNight);
        float groundB = lerpF(40.0f, 15.0f, smoothNight);

        glColor3f(groundR / 255.0f, groundG / 255.0f, groundB / 255.0f);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, -0.15f);
        glVertex2f(-1.0f, -0.15f);
        glEnd();
    }
    // ── AUTUMN SEASON (orange sky, browner ground) ── [Emad]
    else if (autumnMode)
    {
        drawAutumnSky();
        drawStars(); // stars still appear at night
        {
            float _sx, _sy;
            getSunPos(_sx, _sy);
            drawSunAt(_sx, _sy);
        }
        drawMoon();
        drawAutumnGround();
    }
    // WINTER SEASON (pale grey sky, white snowy ground) [Shajmin]
    else if (winterMode)
    {
        drawWinterSky();
        drawStars();
        {
            float _sx, _sy;
            getSunPos(_sx, _sy);
            drawSunAt(_sx, _sy);
        }
        drawMoon();
        drawWinterGround();
    }
    // ── SUMMER SEASON (vivid blue sky, lush green ground) ──
    else if (summerMode)
    {
        drawSummerSky();
        drawStars();
        {
            float _sx, _sy;
            getSunPos(_sx, _sy);
            drawSunAt(_sx, _sy);
        }
        drawMoon();
        drawSummerGround();
    }
    // ── SPRING MODE or NORMAL (sun, moon, stars, dynamic sky) ──
    else
    {
        // 1. Sky — dynamic day/night gradient
        drawSkyGradient();
        drawStars();

        // 2. Sun / Moon
        {
            float _sx, _sy;
            getSunPos(_sx, _sy);
            drawSunAt(_sx, _sy);
        }
        drawMoon();
        // 2.5 Draw clouds (spring only) ← ADD THIS SECTION
        drawSpringClouds();

        // 3. Ground — dynamic colour
        drawDayNightGround();
    }

    // 4. Road, football field, basketball court [Prottoy]
    drawRoadAndPlayground();

    // Winter snow on top of the road / footpath area [Shajmin]
    if (winterMode)
    {
        drawWinterRoadSnow();
    }

    // 5. Brick footpath [Prottoy]
    drawBrickFootpath();

    // 5c. Cars and pedestrians
    drawTraffic();

    // 5d. Static sports field players
    drawSportsPlayers();

    // 5b. Resting petals
    for (int i = 0; i < MAX_RESTING; i++)
    {
        drawSakuraPetal(restingPetals[i].x,
                        restingPetals[i].y,
                        restingPetals[i].size,
                        restingPetals[i].angle,
                        restingPetals[i].alpha,
                        restingPetals[i].colorType);
    }

    // 6. Roadside trees and lamp posts
    drawRoadsideItems();

    // 6b. Falling petals (only in spring mode, suppressed in all other seasons)
    if (springMode && !rainMode)
    {
        for (int i = 0; i < MAX_FALLING; i++)
        {
            if (fallingPetals[i].active)
            {
                float alpha = (fallingPetals[i].y < -0.725f) ? 0.65f : 0.85f;
                drawSakuraPetal(fallingPetals[i].x,
                                fallingPetals[i].y,
                                fallingPetals[i].size,
                                fallingPetals[i].angle,
                                alpha,
                                fallingPetals[i].colorType);
            }
        }
    }

    drawPedestrians();

    // ── Seasonal scene tint (after trees, before buildings) ──
    // Each tint shifts tree and ground colours to match the season
    // without touching any individual drawing function.
    if (autumnMode)
    {
        drawAutumnTint(); // [Emad]
    }
    if (winterMode)
    {
        drawWinterTint(); // [Shajmin]
    }
    if (summerMode)
    {
        drawSummerTint();
        drawSummerHeatHaze();
        // Extra summer decorations
        drawCoconutStall();
        drawKidsEatingMango();
        drawBirdsResting();
        drawDryGroundPatches();
        drawWavyHeatHaze();
    }

    // 7. Left background buildings
    drawLeftBackgroundBuildings();
    drawLeftBackgroundLights();

    // 8. Right background buildings
    drawRightBackgroundBuildings();

    // 9. Annex-9
    drawAnnex9();
    drawAnnex9Lights();

    // 10. C-Building
    drawCBuilding();

    // 11. D-Building
    drawDCurtainLightUnder();
    drawDBuilding();
    drawBuildingLights();

    // ── Rain drops (only in rainy mode, drawn on top of everything) ──
    if (rainMode)
    {
        // Raindrop color slightly affected by night (darker at night)
        float db = getDayBlend();
        float nightFactor = 1.0f - db;
        unsigned char rainR = (unsigned char)(100 - nightFactor * 50);
        unsigned char rainG = (unsigned char)(180 - nightFactor * 80);
        unsigned char rainB = (unsigned char)(255 - nightFactor * 155);

        for (int i = 0; i < MAX_RAIN; i++)
        {
            glPushMatrix();
            glTranslatef(rainX[i], rainY[i], 0.0f);
            glColor3ub(rainR, rainG, rainB);
            glLineWidth(1.5f);
            glBegin(GL_LINES);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(0.003f, -0.03f);
            glEnd();
            glPopMatrix();
        }
    }

    // ── Falling leaves (only in autumn mode, drawn on top of everything) ── [Emad]
    if (autumnMode)
    {
        drawAutumn();
    }

    // Falling snow (only in winter mode, drawn on top of everything) [Shajmin]
    if (winterMode)
    {
        drawWinter();
    }

    // ── FIREWORKS (drawn last, on top of everything) ──
    drawThunderstorm(); // [Sadia] — lightning + flash + impact
    drawFWFlash();
    drawFireworks();

    glFlush();
}
// ================================================================
//  INIT & MAIN
// ================================================================
void init()
{
    glClearColor(0.118f, 0.392f, 0.745f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    // Initialize rain particles
    for (int i = 0; i < MAX_RAIN; i++)
    {
        rainX[i] = -1.0f + (float)(rand() % 200) / 100.0f;
        rainY[i] = -1.0f + (float)(rand() % 200) / 100.0f;
        rainSpeed[i] = 0.008f + (float)(rand() % 15) / 500.0f;
    }

    initPetals();
    initRoadTraffic();
    initSpringClouds(); // [Prottoy]

    // Season + player setup
    initAutumn();    // [Emad]
    initWinter(); // [Shajmin]
    initPlayers(); // [Shajmin]
    initFireworks(); // Initialize fireworks system
}
void update(int value)
{
    updateDayNight();

    // Wind sway runs in ALL seasons — always update it
    windAngle += breezeSpeed;
    windSway = windSwayMax * sinf(windAngle);
    if (windAngle > PI * 2.0f)
    {
        windAngle = 0.0f;
        windDir = (petalRand() > 0.5f) ? 1.0f : -1.0f;
        windSwayMax = 5.0f + petalRand() * 6.0f;
    }

    // Petals only update in spring mode
    if (springMode && !rainMode)
    {
        updatePetals();
    }

    updateTraffic();
    updateSpringClouds();

    // Update rain particles
    if (rainMode)
    {
        for (int i = 0; i < MAX_RAIN; i++)
        {
            rainY[i] -= rainSpeed[i] * rainSpeedMult; // [Sadia] adjustable via p/o
            if (rainY[i] < -1.0f)
            {
                rainY[i] = 1.0f;
                rainX[i] = -1.0f + (float)(rand() % 200) / 100.0f;
            }
        }
    }

    // Autumn — falling leaves (Emad)
    if (autumnMode)
    {
        updateAutumn();
    }

    // Winter — falling snow (Shajmin)
    if (winterMode)
    {
        updateWinter();
    }

    // Move the bouncing players (Shajmin)
    updatePlayers();

    //  Parking arm easing animation [Shajmin]
    {
        float diff = parkingArmTarget - parkingArmAngle;
        if (fabsf(diff) > 0.25f)
            parkingArmAngle += diff * 0.12f; // 12% of the gap per tick [Shajmin]
        else
            parkingArmAngle = parkingArmTarget; // snap when essentially there
    }

    // ── Update fireworks (55ms per tick) ──
    updateFireworks(0.055f);
    updateThunderstorm(0.055f); // [Sadia]

    glutPostRedisplay();
    glutTimerFunc(55, update, 0);
}
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1400, 800);
    glutCreateWindow("AIUB Campus – Complete Scene (All Seasons + Fireworks)");
    glutKeyboardFunc(keyboard);
    init();
    glutDisplayFunc(display);
    glutTimerFunc(20, update, 0);
    glutMainLoop();

    return 0;
}
