# Automatic Runway Assignement System v2

<img width="1000" height="499" alt="image" src="https://github.com/user-attachments/assets/84f9f70c-03fe-4cdc-b7e4-91f5854edea8" />


## Introduction
This software is used to assign automatically euroscope runways according to a database referencing preferential runway as well as using real-time weather data.

## Installation
To install and run this project, follow these steps:

1. Install the software using the setup.exe file
2. In order to use the real weather data, you will need an API Token, head to https://info.avwx.rest/ and create an account, then request an API token
3. Launch ARAS executable, launching it for the first time will create a default config
4. Enter your token inside the input box, a confirmation message should indicate that it has been correctly saved
5. Select your `.rwy` file inside LFXX's euroscope folder (it will be saved for future use)
6. Select the FIR where you want to auto-assign runways and click `Assign Runway`
7. Launch Euroscope!

## Usage
When opening the program, the status info panel will show you the state of required parameters for it to work.<br>
When red, the program could not locate the data, green indicating that it has. <br>
For token status, when entering a new token for the first time, it will be saved but not verified, and will appear yellow.<br>
After you've completed the first auto runway assignement successfully, the token will be verified and valid, the indication will then turn green.<br>
To automatically assign runway, the sofware must be run BEFORE opening Euroscope.<br>
The FIR Airports menu let you customise which airport are being assigned when clicking on a FIR auto-assign button (blue button).<br>
To modify the default list, select the FIR in the dropdown menu, then, the list of current auto-assigned airport will show in the textfield next to it.<br>
Modify this list with ICAO codes separated by comma. You can then add or remove airports according to your requirements.<br>
Clicking the reset button will revert to default list of airports for the currently selected FIR.<br>
Note: if you are adding unsopported ICAO, you will need to also add them using the Settings Window<
      the has4runway "FIR" is used to define airports equipped with 2 departures runways as well as 2 arrival one. Edit this list the same way as the others
      using a texteditor on Documents/ARAS/rwydata.<br>

      Unless specified in the update, you can keep your settings, to do so, proceed as follow:
      Before reinstalling ARAS, make sure to save both rwydata.json & config.json in an other folder, 
      after the reinstall, launch ARAS, it will create those 2 files back, you can replace them with your old config.


## Author
Alexis Balzano
