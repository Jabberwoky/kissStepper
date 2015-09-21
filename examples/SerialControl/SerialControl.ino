/*

Control an Easy Driver through a serial connection

Written by Rylee Isitt
September 14, 2015

The code in this file is released into the public domain.
Libraries are licensed separately (see their licenses for details).

*/
#include <kissStepper.h>

// instantiate an instance of the kissStepper class
// you can leave the MS1 and MS2 pins out if you don't want to control them with software (eg, if you have them hardwired or set with a DIP switch)
// you can also set ENABLE to 255 if you aren't controlling the ENABLE pin with software (eg, if you just pull it to ground)
// The numbers below are the pins for ENABLE, DIR, STEP, MS1, and MS2, respectively
kissStepper mot(7, 3, 4, 5, 6);
// kissStepper mot(7, 3, 4); // if you don't want to programmatically set MS1 and MS2
// kissStepper mot(255, 3, 4); // if you don't want to programmatically set the ENABLE pin either

void loop(void)
{

	// this line below actually makes the motor move
	// and needs to be run repeatedly and often for smooth motion
	// you can do things this way, or you can put work() in a for/while loop
	// or you can call work() from a timer interrupt
	bool moving = mot.work();
	
	
	// get commands from serial
	static String command = "";
	static bool commandIn = false;
	while (Serial.available() > 0) {
		char c = Serial.read();
		if (c == '<') commandIn = true;
		else if (c == '>')
		{
			commandIn = false;
			command.trim();
			command.toLowerCase();
			byte splitAt = command.indexOf(' ');
			String key = command.substring(0, splitAt);
			String value = command.substring(splitAt+1);
			command = "";

			if (key == F("getpos"))
			{
				Serial.print(F("Position: "));
				Serial.println(String(mot.getPos()));
			}
			else if (key == F("target"))
			{
				Serial.print(F("Target: "));
				Serial.println(String(mot.getTarget()));
			}
			else if (key == F("ismoving"))
			{
				if (moving) Serial.println(F("TRUE"));
				else Serial.println(F("FALSE"));
			}
			else if (key == F("goto"))
			{
				int32_t targetPos = value.toInt();
				mot.moveTo(targetPos);
				Serial.print(F("Target: "));
				Serial.println(String(targetPos));
			}
			else if (key == F("move"))
			{
				if (value == F("forward"))
				{
					mot.moveTo(mot.forwardLimit);
					Serial.println(F("Moving forward"));
				}
				else if (value == F("backward"))
				{
					mot.moveTo(mot.reverseLimit);
					Serial.println(F("Moving backward"));
				}
				else
				{
					Serial.print(F("Unrecognized direction:"));
					Serial.println(value);
				}
			}
			else if (key == F("stop"))
			{
				mot.stop();
				Serial.println(F("Motor stopped"));
			}
			else if (key == F("hardstop"))
			{
				mot.hardStop();
				Serial.println(F("Motor stopped"));
			}
			else if (key == F("getmaxspeed"))
			{
				Serial.print(F("Max Speed: "));
				Serial.print(String(mot.getMaxRPM()));
				Serial.println(F(" RPM"));
			}
			else if (key == F("setmaxspeed"))
			{
				word newSpeed = value.toInt();
				Serial.print(F("Max Speed: "));
				Serial.print(String(newSpeed));
				Serial.println(F(" RPM"));
				mot.setMaxRPM(newSpeed);
			}
			else if (key == F("getcurspeed"))
			{
				Serial.print(F("Current Speed: "));
				Serial.print(String(mot.getCurRPM()));
				Serial.println(F(" RPM"));
			}
			else if (key == F("getaccel"))
			{
				Serial.print(F("Accel: "));
				Serial.print(String(mot.getAccel()));
				Serial.println(F(" RPM/m"));
			}
			else if (key == F("setaccel"))
			{
				word newAccel = value.toInt();
				bool result = mot.setAccel(newAccel);
				if (result) {
					Serial.print(F("Accel: "));
					Serial.print(String(newAccel));
					Serial.println(F(" RPM/m"));
				}
				else {
					Serial.println(F("Unable to change acceleration while accelerating"));
				}
				
			}
			else if (key == F("getmode"))
			{
				Serial.print(F("Mode: "));
				switch (mot.getDriveMode())
				{
					case kissStepper::FULL:
						Serial.println(F("full step"));
						break;
					case kissStepper::HALF:
						Serial.println(F("1/2 step"));
						break;
					case kissStepper::QUARTER:
						Serial.println(F("1/4 step"));
						break;
					case kissStepper::EIGHTH:
						Serial.println(F("1/8 step"));
						break;
				}
			}
			else if (key == F("setmode"))
			{
				Serial.print(F("Mode: "));
				if (value == "1")
				{
					Serial.println(F("full step"));
					mot.setDriveMode(kissStepper::FULL);
				}
				else if (value == "2")
				{
					Serial.println(F("1/2 step"));
					mot.setDriveMode(kissStepper::HALF);
				}
				else if (value == "4")
				{
					Serial.println(F("1/4 step"));
					mot.setDriveMode(kissStepper::QUARTER);
				}
				else if (value == "8")
				{
					Serial.println(F("1/8 step"));
					mot.setDriveMode(kissStepper::EIGHTH);
				}
			}
			else if (key == F("isenabled"))
			{
				if (mot.isEnabled()) Serial.println(F("TRUE"));
				else Serial.println(F("FALSE"));
			}
			else if (key == F("enable"))
			{
				mot.enable();
				Serial.println(F("Motor enabled"));
			}
			else if (key == F("disable"))
			{
				mot.disable();
				Serial.println(F("Motor disabled"));
			}
			else
			{
				Serial.print(F("Unrecognized command:"));
				Serial.println(key);
			}
		}
		else if (commandIn) command += c;
	}
}

void setup(void)
{

	// initialize the kissStepper
	// the motor has 200 steps, use 1/8th drive mode, and set the maximum speed to 60 RPM
	mot.begin(200, kissStepper::EIGHTH, 60); 

	Serial.begin(9600);
	
	// send instructions to serial
	Serial.println("");
	Serial.println(F("Usage:"));
	Serial.println(F("<getpos>            returns the current motor position"));
	Serial.println(F("<target>            returns the target position"));
	Serial.println(F("<ismoving>          returns true if the motor is currently moving"));
	Serial.println(F("<goto x>            sends the motor to position x, -2147483648 <= x <= 2147483647"));
	Serial.println(F("<move forward>      continuously move the motor fowards"));
	Serial.println(F("<move backward>     continuously move the motor backwards"));
	Serial.println(F("<stop>              stops the motor, decelerating if acceleration is enabled"));
	Serial.println(F("<hardstop>          stops the motor without decelerating"));
	Serial.println(F("<getmaxspeed>       returns the maximum speed of the motor (in RPM)"));
	Serial.println(F("<setmaxspeed x>     sets the maximum speed of the motor (in RPM), x <= 65535"));
	Serial.println(F("<getcurspeed>       returns the current speed of the motor (in RPM)"));
	Serial.println(F("<getaccel>          returns the acceleration (in RPM/s)"));
	Serial.println(F("<setaccel x>        sets the acceleration (in RPM/s), x <= 65535"));
	Serial.println(F("<getmode>           returns the drive mode of the motor"));
	Serial.println(F("<setmode x>         sets the motor's drive mode, 1 = full step, 2 = 1/2 step, 4 = 1/4 step, 8 = 1/8 step"));
	Serial.println(F("<isenabled>         returns true if the motor is enabled"));
	Serial.println(F("<enable>            enables the motor"));
	Serial.println(F("<disable>           disables the motor"));
	Serial.println("");

}
