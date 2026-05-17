#Computer Graphics Project — AIUB Campus Recreation
A collaborative OpenGL/GLUT project recreating the AIUB campus, built by Prottoy, Emad, Shajmin, Sadia, and Zinedine.


#👤 Contribution Log — Emad
🍂 Seasonal Development — Autumn

Designed and implemented the Autumn season scenario, activated by pressing 'a'. The system simulates 95 independently falling leaves with randomised speeds and four authentic autumn colours (orange, yellow, brown, red), each tracked through its own position and lifetime.

Engineered a warm autumn sky gradient that lerps from deep orange at the top down to a golden horizon, and which correctly darkens at night so the day/night cycle continues to work while the season is active.

Implemented a dry-brown ground tint and a translucent orange canvas overlay that shifts the trees and roadside foliage into an autumn palette automatically, without requiring any modification to the existing tree, road, or building draw functions.

🎮 Dynamic Transformation System

Built the project's required Translation, Scaling, and Rotation transformations using OpenGL's matrix stack (glPushMatrix, glTranslatef, glRotatef, glScalef, glPopMatrix) so that all three transformations are demonstrated through the proper transformation pipeline rather than through manual coordinate multiplication.

Translation — Real-time control over the speed of every car on the road. Cars can be sped up to 5× the default rate or slowed all the way down to a complete stop.

Scaling — Real-time control over the size of every falling autumn leaf, applied via glScalef. Each leaf is drawn around the origin and then transformed, so the scaling pivots around the leaf's own centre rather than around the world origin.

Rotation — Each falling leaf rotates around its own centre via glRotatef, with a randomised per-leaf spin speed and direction (some clockwise, some counter-clockwise) so the falling motion looks natural rather than mechanical.

🔧 Cross-Platform Compatibility

Refactored the project's platform-specific code so the same main.cpp compiles on both macOS and Windows from a single source. The GLUT/OpenGL headers and the audio playback for both the firework sound and the rain sound are now wrapped in #ifdef _WIN32 / #elif __APPLE__ blocks — using MCI and PlaySound on Windows, and system("afplay …") on macOS — so neither platform breaks the build for the other.

🎮 Dynamic Interactivity & Controls
Feature                            Key Binding
Autumn season                  ->  A
Car translation (slower/faster) -> , and .
Leaf scaling (smaller/larger)   -> K and L
Leaf rotation                  ->  Automatic (each leaf has its own spin)


#👤 Contribution Log — Prottoy
🏛️ Architectural Development

Designed and implemented the D-Building of the AIUB Campus Scene, serving as the primary academic structure of the simulation.

🛣️ Environmental & Landscape Design

Constructed the road network and footpaths with accurate proportions relative to the campus layout.
Populated roadsides with a diverse tree ecosystem including Oak, Blossom, and White Blossom tree variants, each built from GL primitives.

⚽ Sports Facilities

Modeled the Football Ground and Basketball Court, adding recreational authenticity to the campus scene.

🌤️ Atmospheric & Lighting System

Engineered the day/night transition system, applying dynamic lighting and color shifts across all scene objects in response to time-of-day cycles.

🍃 Particle & Nature Effects

Implemented a petal falling effect simulating blossom petals drifting from Blossom trees, enhancing environmental realism.

🎮 Dynamic Interactivity & Controls
Feature                            Key Binding
Tree rotation / breeze effect  ->  [- and =] or [_  and +]
Cloud scaling                  ->  Z and X
Cloud movement speed           ->  [ and ]
Firework display               ->  F

🎆 Visual Effects

Developed an elaborate firework animation system as a celebratory visual effect, triggered interactively via keyboard input.

And lastly created the 'Spring' scenario for the project.


#👤 Contribution Log — Shajmin
❄️ Seasonal Development — Winter

Designed and implemented the Winter season scenario, activated by pressing 'w'. The system simulates 200 falling snowflakes with individually randomised speeds, a cool winter sky gradient, a snow-tinted ground, a translucent white canvas overlay that frosts the trees and buildings, and a layer of accumulated snow rendered over the road and footpath.

🏢 Architectural Development

Modeled the Annex-9 foreground building, including its windows, doors, and roof details, contributing one of the major academic structures to the campus layout.

🚗 Parking Lot Scene

Built the parking lot in front of Annex-9, including a set of static parked cars filling the bays and a security booth on the left side of the lot to give the scene a more lived-in feel.

⛹️ Dynamic Sports Players

Implemented moving sports players that bounce around inside the courts — 4 basketball players on the basketball court and 6 football players on the football field. Each player carries its own position and velocity, reverses on wall contact, and updates every frame to give the sports facilities a lively, animated feel.


#👤 Contribution Log — Sadia
🌧️ Seasonal Development — Rainy

Designed and implemented the Rainy season scenario, activated by pressing 'r'. The system simulates 200 falling rain particles with randomised speeds, a grey overcast sky, a wet ground tint, and pedestrians carrying umbrellas to sell the change in weather.

🏢 Architectural Development

Contributed the Basundhara background buildings, helping fill out the right side of the skyline and adding depth to the overall campus scene.

⛈️ Thunderstorm System

Added a thunderstorm effect, activated by pressing 't'. Each press of 't' cycles the storm intensity — level 1 (single small bolt), level 2 (medium bolt with one branch), level 3 (large bolt with three branches) — and a fourth press turns the storm off entirely. Each lightning bolt is generated procedurally as a jagged segmented line, scaled into existence using glScalef so it appears to grow out of the sky, accompanied by a full-screen flash and a ground impact glow.

🔊 Audio Effects

Added a thunderstorm sound effect and a continuous rain sound that begins automatically when the rainy season is selected and stops cleanly when any other season is chosen.

🎮 Dynamic Interactivity & Controls
Feature                            Key Binding
Rainy season                   ->  R
Thunderstorm intensity cycle   ->  T
Rain speed up                  ->  P
Rain slow down                 ->  O


#📁 Build Notes
macOS:   g++ main.cpp -framework OpenGL -framework GLUT -o aiub_campus
Windows: link against freeglut + opengl32 + winmm.lib

Audio files expected next to the executable: fireWorkSoundV3.wav, rain.wav
