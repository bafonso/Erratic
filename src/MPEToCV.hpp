#include <list>
#include <algorithm>
//#include "rtmidi/RtMidi.h"
#include "MPE.hpp"
#include "MidiIO.hpp"
#include "dsp/digital.hpp"

using namespace rack;
/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod wheel to
 * CV
 */
struct MidiValue {
	int val = 0; // Controller value
	TransitionSmoother tSmooth;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MPEToCVInterface : MidiIO, Module {
	enum ParamIds {
		RESET_PARAM,
		NUM_PARAMS,
        BEND_RANGE_PARAM
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		PITCH_OUTPUT = 0,
		GATE_OUTPUT,
		VELOCITY_OUTPUT,
		MOD_OUTPUT,
		PITCHWHEEL_OUTPUT,
		CHANNEL_AFTERTOUCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RESET_LIGHT,
		NUM_LIGHTS
	};

    int bendRange = 2; // our default
	std::list<int> notes;
	bool pedal = false;
	int note = 60; // C4, most modules should use 261.626 Hz
	int vel = 0;
	MidiValue mod;
	MidiValue afterTouch;
	MidiValue pitchWheel;
	bool gate = false;

	SchmittTrigger resetTrigger;

	MPEToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		pitchWheel.val = 64;
		pitchWheel.tSmooth.set(0, 0);
	}

	~MPEToCVInterface() {
	};

	void step() override;

	void pressNote(int note);

	void releaseNote(int note);

	void processMidi(std::vector<unsigned char> msg);

	json_t *toJson() override {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		baseFromJson(rootJ);
	}

	void reset() override {
		resetMidi();
	}

	void resetMidi() override;

};

void MPEToCVInterface::resetMidi() {
	mod.val = 0;
	mod.tSmooth.set(0, 0);
	pitchWheel.val = 64;
	pitchWheel.tSmooth.set(0, 0);
	afterTouch.val = 0;
	afterTouch.tSmooth.set(0, 0);
	vel = 0;
	gate = false;
	notes.clear();
}

void MPEToCVInterface::step() {
	if (isPortOpen()) {
		std::vector<unsigned char> message;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		getMessage(&message);
		while (message.size() > 0) {
			processMidi(message);
			getMessage(&message);
		}
	}


	if (resetTrigger.process(params[RESET_PARAM].value)) {
		resetMidi();
		return;
	}

	lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / 0.55 / engineGetSampleRate(); // fade out light

	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	outputs[VELOCITY_OUTPUT].value = vel / 127.0 * 10.0;

	int steps = int(engineGetSampleRate() / 32);

	if (mod.changed) {
		mod.tSmooth.set(outputs[MOD_OUTPUT].value, (mod.val / 127.0 * 10.0), steps);
		mod.changed = false;
	}
	outputs[MOD_OUTPUT].value = mod.tSmooth.next();

	if (pitchWheel.changed) {
		pitchWheel.tSmooth.set(outputs[PITCHWHEEL_OUTPUT].value, (pitchWheel.val - 64) / 64.0 * 1, steps);
		pitchWheel.changed = false;
	}

	// outputs[PITCHWHEEL_OUTPUT].value = pitchWheel.tSmooth.next();

    float BendableSemitones = 2;
	outputs[PITCH_OUTPUT].value = (((note - 60)) / 12.0) + ((pitchWheel.val - 8192 ) / 8192.0 / 12.0 * BendableSemitones ) ;





	/* NOTE: I'll leave out value smoothing for after touch for now. I currently don't
	 * have an after touch capable device around and I assume it would require different
	 * smoothing*/
	outputs[CHANNEL_AFTERTOUCH_OUTPUT].value = afterTouch.val / 127.0 * 10.0;
}

void MPEToCVInterface::pressNote(int note) {
	// Remove existing similar note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);
	// Push note
	notes.push_back(note);
	this->note = note;
	gate = true;
}

void MPEToCVInterface::releaseNote(int note) {
	// Remove the note
	auto it = std::find(notes.begin(), notes.end(), note);
	if (it != notes.end())
		notes.erase(it);

	if (pedal) {
		// Don't release if pedal is held
		gate = true;
	} else if (!notes.empty()) {
		// Play previous note
		auto it2 = notes.end();
		it2--;
		this->note = *it2;
		gate = true;
	} else {
		gate = false;
	}
}

void MPEToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf;
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];

	// fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

	// Filter channels
	if (this->channel >= 0 && this->channel != channel)
		return;

	switch (status) {
		// note off
		case 0x8: {
			releaseNote(data1);
		}
			break;
		case 0x9: // note on
			if (data2 > 0) {
				pressNote(data1);
				this->vel = data2;
			} else {
				// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
				releaseNote(data1);
			}
			break;
		case 0xb: // cc
			switch (data1) {
				case 0x01: // mod
					mod.val = data2;
					mod.changed = true;
					break;
				case 0x40: // sustain
					pedal = (data2 >= 64);
					if (!pedal) {
						releaseNote(-1);
					}
					break;
			}
			break;
		case 0xe: // pitch wheel, must combine two 7 bit in two bytes into a 14bit msg
            {
            int nBytes, i;
            double stamp;
            nBytes = msg.size();
            for ( i=0; i<nBytes; i++ )
                std::cout << "Byte " << i << " = " << (int)msg[i] << ", ";
            if ( nBytes > 0 )
                std::cout << "stamp = " << stamp << std::endl;
    
            // We want 2 bytes but variable size may change with platform, maybe we should do a more robust way
            uint16_t twoBytes ; // Initialize our final pitchWheel variable.
            // we don't need to shift the first byte because it's 7 bit (always starts with 0)
            twoBytes = ( (uint16_t)msg[2] << 7) | ( (uint16_t)msg[1] ) ;
            // std::cout << sizeof(int) << std::endl;
            std::bitset<8> msgOne(msg[1]);
            std::bitset<8> msgTwo(msg[2]);
            
            std::bitset<16> x(twoBytes);
            std::cout << "msg[1] and 2 are " << msgOne << " " << msgTwo << " and shifted is " << x << std::endl;
            
            std::cout << "twoBytes is " << (int)twoBytes << std::endl;

			pitchWheel.val = twoBytes;
			pitchWheel.changed = true;
            }
			break;
		case 0xd: // channel aftertouch
			afterTouch.val = data1;
			afterTouch.changed = true;
			break;
	}
}

