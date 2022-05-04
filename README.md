# scrcpy-with-audio
Display and control your Android device. This fork hopefully simplifies things by including a series of batch and VBS scripts to easily run scrcpy for casting/control with sndcpy for audio.

## Enable "Developer Options" on your phone.
This is a bit different for each phone. Google "how to enable developer options for [your phone]".
Then under Developer options, do the following:
- Enable "`USB Debugging`"
- If you see it in your phone's settings, also enable "`USB Debugging (Security Settings)`"
  - This is for keyboard/mouse support.

## Just for initial setup, connect your phone to your PC with USB
- In your settings, you should see something like "USB Preferences"
  - Make sure __"`File transfer / Android Auto`"__ is selected. The wording may be a bit different on your phone.

## Download the latest version and simply open "`Start Phone`"
- If you want to connect over Wifi, just type your phone's IP address into the `phone-connection.config` file. 
  - _Your phone's IP address can be found in your phone settings_  

-------------------------
-------------------------
-------------------------
# How to Install Releases from Original Creator 
### (because trusting strangers on the internet is scary)
- Download a trusted release from the creator's page [here](https://github.com/Genymobile/scrcpy).
- Extract both scrcpy and sndcpy to the `src` folder and replace any existing files.
