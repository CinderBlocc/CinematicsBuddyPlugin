# Cinematics Buddy Plugin

*If you are reading this file in notepad, I would recommend going to [this link](https://github.com/CinderBlocc/CinematicsBuddyPlugin/tree/master/bakkesmod/data/CinematicsBuddy) to view the formatted readme page.*

## OVERVIEW
CinematicsBuddy is a tool designed to make life easier for editors and spectators. It provides features for capturing camera / ball / car animation, importing camera animations, and input overrides for more refined control over the camera with controllers or keyboard and mouse. Below are the instructions for each part of the plugin. For instructions about third party scripts (such as 3ds Max and After Effect's importers), refer to their respective readme files listed below.
- [3ds Max - PENDING](https://github.com/CinderBlocc/CinematicsBuddyMaxscript/tree/master/bakkesmod/data/CinematicsBuddy/Plugins/3dsMax)
- [After Effects - PENDING](https://github.com/CinderBlocc/CinematicsBuddyAE/tree/main/bakkesmod/data/CinematicsBuddy/Plugins/AfterEffects)


## EXPORTING

###### NORMAL RECORDING
Pending.

###### BUFFER RECORDING
Pending.


## IMPORTING
Pending.


## CAMERA OVERRIDES
Each of the settings follows this format: `Setting name` *(default value, minimum, maximum)* Setting description.

###### Checkboxes
- `Enable Overrides` *(false, -, -)* Globally enables or disables the camera overrides feature
- `Local Momentum Preservation` *(false, -, -)* Camera either maintains linear momentum, or follows the direction the camera is facing.
- `Local Movement` *(true, -, -)* Camera moves according to its local axes. For instance if you are looking downward and press forward, the camera will move in the direction you are looking.
- `Local Rotation` *(false, -, -)* Camera rotates along its local axes. This effect is particularly noticable when the camera is rolled more than 45 degrees and pitch or yaw inputs are given.
- `Invert Pitch (Controller)` *(false, -, -)* Inverts the pitch inputs from the controller. Does not affect mouse input.
- `Hard Floors` *(true, -, -)* Prevents the camera from going below the floor. The floor is specified by the **Floor Height** variable.

###### Sliders
- `Floor Height` *(10, -50, 50)* When **Hard floors** is enabled, this determines how low the camera can go.
- `Movement Speed` *(1, 0, 5)* Max speed of camera linear velocity.
- `Movement Acceleration` *(1, 0, 5)* How long it takes to reach max speed. A higher number will reach max speed faster.
- `Rotation Speed (Mouse)` *(1, 0, 3)* Max speed of mouse rotation. These rotations are defined by the mouse's delta movement.
- `Rotation Speed (Non-Mouse)` *(1, 0, 3)* Max speed of non-mouse rotation. These are rotations defined by a rate of rotation. Keyboard inputs also count toward this.
- `Rotation Acceleration (Mouse)` *(1, 0, 10)* How long it takes mouse rotation to reach max speed. *NOTE: really high acceleration values cause large rotation stutters.*
- `Rotation Acceleration (Controller)` *(1, 0, 10)* How long it takes controller rotation to reach max speed.
- `FOV Rotation Scale` *(.3, 0, 1)* Multiplier for rotation speed as FOV zooms in. The lower this number, the slower the rotation becomes when you zoom in.
- `FOV Minimum` *(20, 5, 170)* The lower limit of FOV.
- `FOV Maximum` *(120, 5, 170)* The upper limit of FOV. *NOTE: If minimum is greater than maximum, it will become the new maximum and vice versa.*
- `FOV Speed` *(1, 0, 3)* Max speed of zoom change.
- `FOV Acceleration` *(1, 0, 10)* How long it takes FOV change to reach max speed.
- `FOV Limit Ease` *(.1, 0, .5)* If current FOV is taken as a percentage between Minimum and Maximum, this specifies how close that percentage must be toward the lower or upper bounds before it starts easing into that limit.

###### Input Swaps
- `Toggle Roll Binding` *(Xbox RB - PS4 R1, -, -)* The button that needs to be held to initiate the **Roll Input Swap**.
- `Roll Input Swap` *(Yaw)* The input to be swapped with roll. By default, when holding the right bumber, left and right on the right analog stick will roll instead of yaw.
- `Toggle FOV Binding` *(Xbox LB - PS4 L1, -, -)* The button that needs to be held to initiate the **FOV Input Swap**.
- `FOV Input Swap` *(Right)* The input to be swapped with FOV. By default, when holding the left bumber, left and right on the left analog stick will zoom in and out instead of move the camera left and right.

###### Configs
- `Current Config` A dropdown menu displaying the available configs. Configs are stored as `.cfg` files in /data/CinematicsBuddy/CameraConfigs/ and any subfolders within that folder. Selecting an option from this dropdown will apply the settings from that config file.
- `Update Config List` Recursively loops through all files in /data/CinematicsBuddy/CameraConfigs/ and its subfolders and adds `.cfg` files to the list.
- `New Config Name` The name given to the new file when you click **Save Config**.
- `Save Config` Saves all camera override settings to a config file. The file's name is specified by **New Config Name** and will be saved in the /data/CinematicsBuddy/CameraConfigs/ folder.