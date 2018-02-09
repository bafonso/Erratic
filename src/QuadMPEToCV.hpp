// #include <list>
#include <algorithm>
//#include "rtmidi/RtMidi.h"
#include "MPE.hpp"
#include "MidiIO.hpp"
#include "dsp/digital.hpp"

//static const int polyphony = 4;

// using namespace rack;

/*
 * MIDIToCVInterface converts midi note on/off events, velocity , channel aftertouch, pitch wheel and mod wheel to
 * CV
 */
struct MidiValue {
	int val = 0; // Controller value
	TransitionSmoother tSmooth;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MidiNote {
	int pitch = 60;
	int vel = 0; // velocity
	bool gate = false;
	int channel ;
	bool noteOn, noteOff ;
	bool changed = false;
};

struct MPEPlusValue {
	uint16_t val = 0; // Controller value
	int MSB = 0 ;
	int LSB = 0;
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MidiPedalValue {
	int val = 0; // Controller value
	int cc ; // need to set it
	bool changed = false; // Value has been changed by midi message (only if it is in sync!)
};

struct MPEChannel { // This contains the required info for each channel, each note in practical terms
	int MIDIChannel; // must initialize
	MidiNote note;
	MidiValue mod;
	MidiValue afterTouch;
	MidiValue pitchWheel;
	MidiValue Yaxis ;
	MPEPlusValue MPEPlusyAxis, MPEPluszAxis;
	bool changed = false;
};

struct QuadMPEToCVInterface : MidiIO, Module {
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
		GATE_OUTPUT = 4,
		VELOCITY_OUTPUT = 8,
		PRESSURE_OUTPUT = 12,
		Y_OUTPUT = 16,
		PEDAL_OUTPUT = 20,
		NUM_OUTPUTS = 21
	};
	enum LightIds {
		RESET_LIGHT,
		VELOCITY_LIGHT,
		PRESSURE_LIGHT,
		Y_AXIS_LIGHT,
		PEDAL_LIGHT,
		NUM_LIGHTS
	};

	int polyphony = 4;
	std::vector<MPEChannel> mpeChannels;
	// MPEChannel mpeChannels[4] ; // using this here instead of a container
	
	bool noteOffReset = true; // Our default
    int baseMIDIChannel = 2 ;
	int bendRange = 48; // our default is 48 (common for ROLI), Continuum defaults to 96. This is a global parameter (for now)
	int globalMIDIChannel = 16; // Our default channel is 16. ROLI users will want to set this is 1
	bool MPEPlus = false ; // This is specially useful for Haken Continuum
	int YaxisCC = 74 ;

	bool pedal = false;
	// int note = 60; // C4, most modules should use 261.626 Hz
	int vel = 0;

	MidiPedalValue midiPedalOne ;

	//bool gate = false;

	SchmittTrigger resetTrigger;

	QuadMPEToCVInterface() : MidiIO(), Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		// for (int p=0; p < polyphony ; p++) {
		// 	pitchWheel[p].val = 64;
		// 	//pitchWheel[p].tSmooth.set(0, 0);
		// }
		mpeChannels.reserve(polyphony);
		midiPedalOne.cc = 12; // By default we use 12 (barrel i on the Haken Continuum)		
		this->setupMIDIChannels();
	}

	~QuadMPEToCVInterface() {
	};

	void step() override;

	void pressNote(MidiNote note);

	void releaseNote(MidiNote note);

	void processMidi(std::vector<unsigned char> msg);

	void setupMIDIChannels();

	void fromJson(json_t *rootJ) override {
		baseFromJson(rootJ);
		json_t *bendRangeJ = json_object_get(rootJ, "bendRange");
		if (bendRangeJ)
			bendRange = json_integer_value(bendRangeJ);
		
		json_t *baseMIDIChannelJ = json_object_get(rootJ, "baseMIDIChannel");
		if (baseMIDIChannelJ)
			baseMIDIChannel = json_integer_value(baseMIDIChannelJ);
		
		json_t *globalMIDIChannelJ = json_object_get(rootJ, "globalMIDIChannel");
		if (globalMIDIChannelJ)
			globalMIDIChannel = json_integer_value(globalMIDIChannelJ);

		json_t *noteOffResetJ = json_object_get(rootJ, "noteOffReset");
		if (noteOffResetJ)
			noteOffReset = json_boolean_value(noteOffResetJ);

		json_t *MPEPlusModeJ = json_object_get(rootJ, "MPEPlusMode");
		if (MPEPlusModeJ)
			MPEPlus = json_boolean_value(MPEPlusModeJ);

		this->setupMIDIChannels();
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		addBaseJson(rootJ);
		// Semitones
		// std::cout<< "We set bendRange to " << bendRange << std::endl;
		json_object_set_new(rootJ, "noteOffReset", json_boolean(noteOffReset));
		json_object_set_new(rootJ, "MPEPlusMode", json_boolean(MPEPlus));
		json_object_set_new(rootJ, "bendRange", json_integer(bendRange));
		json_object_set_new(rootJ, "baseMIDIChannel", json_integer(baseMIDIChannel));
		json_object_set_new(rootJ, "globalMIDIChannel", json_integer(globalMIDIChannel));
		return rootJ;
	}


	void reset() override {
		resetMidi();
	}

	void resetMidi() override;

};

void QuadMPEToCVInterface::setupMIDIChannels() {
	// std::cout << " Setting up MIDI channels with baseMIDIChannel set to " << baseMIDIChannel << std::endl;
	for (int p=0 ; p < polyphony ; p++) {
		// std::cout << " p MIDIChannel " << p << " " << p + baseMIDIChannel - 1 << std::endl;
		mpeChannels[p].MIDIChannel = p + baseMIDIChannel - 1; // MPE channels start at 2 onwards. We are using MIDI channel starting at 0
	}
}

void QuadMPEToCVInterface::resetMidi() {
	for (int p=0; p < polyphony ; p++) {
		mpeChannels[p].mod.val = 0;
		mpeChannels[p].pitchWheel.val = 0;
		mpeChannels[p].afterTouch.val = 0;
	}



	// mod.val = 0;
	// mod.tSmooth.set(0, 0);
	// pitchWheel.val = 64;
	// pitchWheel.tSmooth.set(0, 0);
	// afterTouch.val = 0;
	// afterTouch.tSmooth.set(0, 0);
	//vel = 0;
	//gate = false;
	//notes.clear();
}

void QuadMPEToCVInterface::step() {
	if (isPortOpen()) {
		std::vector<unsigned char> message;
		int msgsProcessed = 0;

		// midiIn->getMessage returns empty vector if there are no messages in the queue
		// Original Midi to CV limits processing to 4 midi msgs, we should log how many we do at a time to look for
		// potential issues, specially with MPE+
		getMessage(&message);
		while (msgsProcessed < 4 && message.size() > 0) {
			processMidi(message);
			getMessage(&message);
			msgsProcessed++;
		}
	}

	// if (resetTrigger.process(params[RESET_PARAM].value)) {
	// 	resetMidi();
	// 	return;
	// }

	// lights[RESET_LIGHT].value -= lights[RESET_LIGHT].value / 0.55 / engineGetSampleRate(); // fade out light

	for (int ci=0; ci < polyphony; ci++) {
		if (mpeChannels[ci].changed) {
			if (mpeChannels[ci].note.changed) {
				// std::cout << "New note on ci " << ci << std::endl;
				// std::cout << "gate is " << mpeChannels[ci].note.gate << std::endl;
				outputs[GATE_OUTPUT+ci].value = mpeChannels[ci].note.gate ? 10.0 : 0.0;
				// std::cout << "outputs[GATE_OUTPUT+ci].value is " << outputs[GATE_OUTPUT+ci].value << std::endl;
				outputs[VELOCITY_OUTPUT+ci].value = mpeChannels[ci].note.vel / 127.f * 10.f;
				outputs[PITCH_OUTPUT+ci].value = (((mpeChannels[ci].note.pitch - 60)) / 12.0) + ((mpeChannels[ci].pitchWheel.val - 8192 ) / 8192.0 / 12.0 * (float)bendRange ) ;
				// std::cout << "outputs[VELOCITY_OUTPUT+ci].value is " << outputs[VELOCITY_OUTPUT+ci].value << std::endl;
				
				if (mpeChannels[ci].note.noteOff && noteOffReset) { // We reset all info when the note goes off
					std::cout << "We execute the note off reset" << std::endl;
					mpeChannels[ci].pitchWheel.val = 0;
					mpeChannels[ci].pitchWheel.changed = false;
					outputs[PITCH_OUTPUT+ci].value = 0 ;
					mpeChannels[ci].afterTouch.val = 0;
					mpeChannels[ci].afterTouch.changed = false;
					outputs[PRESSURE_OUTPUT+ci].value = 0 ;
					mpeChannels[ci].Yaxis.val = 0;
					mpeChannels[ci].Yaxis.changed = false;
					outputs[Y_OUTPUT+ci].value = 0 ;
					mpeChannels[ci].note.noteOff = false;
				}
				mpeChannels[ci].note.changed = false;
			}
			if (mpeChannels[ci].note.gate) {
				if (mpeChannels[ci].pitchWheel.changed && mpeChannels[ci].note.gate ) {
					// std::cout << "mpeChannels[ci].pitch is " << mpeChannels[ci].note.pitch << std::endl;
					outputs[PITCH_OUTPUT+ci].value = (((mpeChannels[ci].note.pitch - 60)) / 12.0) + ((mpeChannels[ci].pitchWheel.val - 8192 ) / 8192.0 / 12.0 * (float)bendRange ) ;
					mpeChannels[ci].pitchWheel.changed = false;
					// std::cout << "Setting pitch on ci " << ci << " to " << outputs[PITCH_OUTPUT+ci].value << std::endl;
				}
				if (MPEPlus) { // We process MPE+ or not here
					if (mpeChannels[ci].MPEPluszAxis.changed) {
						// Combine two 7 bit into 14bit
						mpeChannels[ci].MPEPluszAxis.val = ( (uint16_t)mpeChannels[ci].MPEPluszAxis.MSB << 7) | ( (uint16_t)mpeChannels[ci].MPEPluszAxis.LSB ) ;
						outputs[PRESSURE_OUTPUT+ci].value = mpeChannels[ci].MPEPluszAxis.val / 16384.0 * 10.f;
						mpeChannels[ci].MPEPluszAxis.changed = false;
						std::cout << "Setting pressure on ci " << ci << " to " << outputs[PRESSURE_OUTPUT+ci].value << std::endl;
					}
					if (mpeChannels[ci].MPEPlusyAxis.changed) {
						// Combine two 7 bit into 14bit
						mpeChannels[ci].MPEPlusyAxis.val = ( (uint16_t)mpeChannels[ci].MPEPlusyAxis.MSB << 7) | ( (uint16_t)mpeChannels[ci].MPEPlusyAxis.LSB ) ;
						outputs[Y_OUTPUT+ci].value = mpeChannels[ci].MPEPlusyAxis.val / 16384.0 * 10.f;
						// std::cout << "Y axis is " << outputs[Y_OUTPUT].value << std::endl;
						mpeChannels[ci].MPEPlusyAxis.changed = false;
					} 
				} else {
					if (mpeChannels[ci].afterTouch.changed ) {
						outputs[PRESSURE_OUTPUT+ci].value = mpeChannels[ci].afterTouch.val / 127.f * 10.f;
						mpeChannels[ci].afterTouch.changed = false;
						// std::cout << "outputs[PRESSURE_OUTPUT+ci].value is " << outputs[PRESSURE_OUTPUT+ci].value << std::endl;
					}
					if (mpeChannels[ci].Yaxis.changed ) {
						outputs[Y_OUTPUT+ci].value = mpeChannels[ci].Yaxis.val / 127.f * 10.f;
						mpeChannels[ci].Yaxis.changed = false;
						// std::cout << "outputs[Y_OUTPUT+ci].value is " << outputs[Y_OUTPUT+ci].value << std::endl;
					}
				}
			}
		
		mpeChannels[ci].changed = false;	
		}
	}

	// Pedal
	if (midiPedalOne.changed) {
		outputs[PEDAL_OUTPUT].value = midiPedalOne.val / 127.f * 10.f ;
		// std::cout << " We set the output outputs[PEDAL_OUTPUT].value to " << outputs[PEDAL_OUTPUT].value << std::endl;
		midiPedalOne.changed = false;
	}
/*
	outputs[GATE_OUTPUT].value = gate ? 10.0 : 0.0;
	outputs[VELOCITY_OUTPUT].value = vel / 127.0 * 10.0;

	// Pressure
	if (MPEPlus) {
		if (MPEPluszAxis.changed) {
			// Combine two 7 bit into 14bit
			MPEPluszAxis.val = ( (uint16_t)MPEPluszAxis.MSB << 7) | ( (uint16_t)MPEPluszAxis.LSB ) ;
			outputs[PRESSURE_OUTPUT].value = MPEPluszAxis.val / 16384.0 * 10.f;
			MPEPluszAxis.changed = false;
		}
		if (MPEPlusyAxis.changed) {
			// Combine two 7 bit into 14bit
			MPEPlusyAxis.val = ( (uint16_t)MPEPlusyAxis.MSB << 7) | ( (uint16_t)MPEPlusyAxis.LSB ) ;
			outputs[Y_OUTPUT].value = MPEPlusyAxis.val / 16384.0 * 10.f;
			std::cout << "Y axis is " << outputs[Y_OUTPUT].value << std::endl;
			MPEPlusyAxis.changed = false;
		}
	} else {  // Standard resolution MPE
		if (afterTouch.changed) {
			outputs[PRESSURE_OUTPUT].value = afterTouch.val / 127.0 * 10;
			afterTouch.changed = false;
		}
		if (Yaxis.changed) {
			outputs[Y_OUTPUT].value = Yaxis.val / 127.0 * 10;
			Yaxis.changed = false;
		}
	}
	

	// 1/V incorporates pitch wheel changes
	if (pitchWheel.changed | this->newNote) {
		outputs[PITCH_OUTPUT].value = (((note - 60)) / 12.0) + ((pitchWheel.val - 8192 ) / 8192.0 / 12.0 * (float)bendRange ) ;
		pitchWheel.changed = false;
		this->newNote = false;
	}	

	*/
}

// void QuadMPEToCVInterface::pressNote(MidiNote note) {
// 	// std::cout << "Oh yeah" << std::endl;
// 	mpeChannels[note.ci].pitch = note.pitch;
// 	mpeChannels[note.ci].newNote = true;
// 	mpeChannels[note.ci].gate = note.gate;
// 	mpeChannels[note.ci].changed = true;
// }

// void QuadMPEToCVInterface::releaseNote(MidiNote note) {
// 	// std::cout << "Oh yeah" << std::endl;
// 	mpeChannels[note.ci].newNote = true;
// 	mpeChannels[note.ci].gate = note.gate;
// 	mpeChannels[note.ci].changed = true;
// }

// void QuadMPEToCVInterface::releaseNote(MidiNote note) {
// 	// Remove existing similar note
// 	mpeChannels[channel].note = 0;
// 	mpeChannels[channel].gate = false;
// 	mpeChannels[channel].changed = true;
// }

// void QuadMPEToCVInterface::releaseNote(int note) {
// 	// Remove the note
// 	auto it = std::find(notes.begin(), notes.end(), note);
// 	if (it != notes.end())
// 		notes.erase(it);

// 	if (pedal) {
// 		// Don't release if pedal is held
// 		gate = true;
// 	} else if (!notes.empty()) {
// 		// Play previous note
// 		auto it2 = notes.end();
// 		it2--;
// 		this->note = *it2;
// 		gate = true;
// 	} else {
// 		gate = false;
// 	}
// }

void QuadMPEToCVInterface::processMidi(std::vector<unsigned char> msg) {
	int channel = msg[0] & 0xf; // starts at 0
	int status = (msg[0] >> 4) & 0xf;
	int data1 = msg[1];
	int data2 = msg[2];
	
	if (status == 0xb && ( data1 == 111 || data1 == 118)) {
		return;
	}

	// fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);

	// Filter only the channels of our polyphony, it must be within our boundaries :)
	// std::cout << "MIDI channel and mpeChannels[0].MIDIChannel " << channel << " " << mpeChannels[0].MIDIChannel << std::endl;

	// std::cout << "polyphony is " << polyphony << std::endl;
	// std::cout << "mpeChannels[0].MIDIChannel is " << mpeChannels[0].MIDIChannel << " and mpeChannels[polyphony].MIDIChannel " 
	// << mpeChannels[polyphony-1].MIDIChannel << std::endl;
	// for (int p=0; p < polyphony ; p++ ) {
	// 	std::cout << " mpeChannels[" << p << "].MIDIChannel: " << mpeChannels[p].MIDIChannel << std::endl;
	// }
	if ( channel >= mpeChannels[0].MIDIChannel && channel <= mpeChannels[polyphony-1].MIDIChannel   ) { // Only process the channel we want
		// std::cout << "We process" << std::endl;
		// std::cout << "channel is " << channel << " baseMIDIChannel is " << baseMIDIChannel << " ci is " << ci << std::endl;
		// start	ci	channel
		// 1		0	0
		// 1		1	1
		// 2		0	1
		// 2		1	2
		// 3		0	2
		// 3		1	3

		int ci = channel - baseMIDIChannel + 1;
		
		switch (status) {
			// note off
			case 0x8: {
				// std::cout << "Note off" << std::endl;
				mpeChannels[ci].note.gate= false;
				mpeChannels[ci].note.vel = data2;
				mpeChannels[ci].note.gate = false;
				mpeChannels[ci].note.noteOff = true;
				mpeChannels[ci].note.changed = true;
				mpeChannels[ci].changed = true;
			}
				break;
			case 0x9: // note on
				// std::cout << "Note on" << std::endl;
				// fprintf(stderr, "channel %d status %d data1 %d data2 %d\n", channel, status, data1, data2);
				// std::cout << "ci is " << ci << std::endl;
				// for (int p=0 ; p < polyphony ; p++) {
				// 	std::cout << " p MIDIChannel " << p << " " << mpeChannels[p].MIDIChannel << std::endl;
				// 	// mpeChannels[p].MIDIChannel = p + baseMIDIChannel - 1; // MPE channels start at 2 onwards. We are using MIDI channel starting at 0
				// }
				if (data2 > 0) { // note ON
					mpeChannels[ci].note.gate= true;
					mpeChannels[ci].note.pitch = data1;
					mpeChannels[ci].note.vel = data2;
					mpeChannels[ci].note.changed = true;
					mpeChannels[ci].changed = true;
				} else { // note off
					// For some reason, some keyboards send a "note on" event with a velocity of 0 to signal that the key has been released.
					mpeChannels[ci].note.gate= false;
					mpeChannels[ci].note.vel = 0;
					mpeChannels[ci].note.noteOff = true;
					mpeChannels[ci].note.changed = true;
					mpeChannels[ci].changed = true;
				}
				break;
			case 0xe: // pitch wheel, we combine two 7 bit in two bytes into a 14bit msg
				{
				
				// We want 2 bytes but variable size may change with platform, maybe we should do a more robust way
				uint16_t twoBytes ; // Initialize our final pitchWheel variable.
				// we don't need to shift the first byte because it's 7 bit (always starts with 0)
				twoBytes = ( (uint16_t)msg[2] << 7) | ( (uint16_t)msg[1] ) ;
				// std::cout << "Pitch wheel " << twoBytes << " on channel and -1 : " << channel << " " << channel -1 << std::endl;
				mpeChannels[ci].pitchWheel.val = twoBytes;
				mpeChannels[ci].changed = true;
				mpeChannels[ci].pitchWheel.changed = true;
				}
				break;
		}
		if (MPEPlus) { // Processing MPE+ data
			// Note from the Haken Continuum Manual:
			// (To avoid the glitches, the synthesizer can do synchronous 14-bit updates with output from the Continuum: 
			// simply save the least significant data, and do not apply it until the most significant data is received.)
			switch (data1) {
				case 74: // Y axis
					mpeChannels[ci].MPEPlusyAxis.MSB = data2;
					mpeChannels[ci].MPEPlusyAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
				case 106:
					mpeChannels[ci].MPEPlusyAxis.LSB = data2;
					mpeChannels[ci].MPEPlusyAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
				case 70: // Z or Pressure
					mpeChannels[ci].MPEPluszAxis.MSB = data2;
					mpeChannels[ci].MPEPluszAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
				case 102:
					mpeChannels[ci].MPEPluszAxis.LSB = data2;
					mpeChannels[ci].MPEPluszAxis.changed = true;
					mpeChannels[ci].changed = true;
					break;
			}
		} else { // Non MPE+ data
			if (status == 0xd) { // Channel Pressure
				// std::cout << " We parse channel aftertouch data that is " << data1 << std::endl;
				mpeChannels[ci].afterTouch.val = data1;
				mpeChannels[ci].afterTouch.changed = true;
				mpeChannels[ci].changed = true;
			}
			if (status == 0xb && data1 == 0x4a) { // CC (oxb) #74 <- we should probably make this assignable if neeed.
				// std::cout << " We parse CC 74 data that is " << data1 << std::endl;
				mpeChannels[ci].Yaxis.val = data2;
				mpeChannels[ci].Yaxis.changed = true;
				mpeChannels[ci].changed = true;
			}
		} // End MPE or MPE+ switch
	} // End note processing

	if (this->globalMIDIChannel == (channel + 1) ) { // If we're on global midi channel
		// std::cout <<"Global channel!" << std::endl;
		if (data1 == midiPedalOne.cc) {
			// std::cout <<"Pedal One value is " << data2 << std::endl;
			midiPedalOne.val = data2;
			midiPedalOne.changed = true;
		}
	}

	/*
		switch (status) {
			case 0xb: // cc
				if (MPEPlus) { // Processing MPE+ data
					// Note from the Haken Continuum Manual:
					// (To avoid the glitches, the synthesizer can do synchronous 14-bit updates with output from the Continuum: 
					// simply save the least significant data, and do not apply it until the most significant data is received.)
					switch (data1) {
						case 74: // Y axis
							MPEPlusyAxis.MSB = data2;
							MPEPlusyAxis.changed = true;
							break;
						case 106:
							MPEPlusyAxis.LSB = data2;
							MPEPlusyAxis.changed = true;
							break;
						case 70: // Z or Pressure
							MPEPluszAxis.MSB = data2;
							MPEPluszAxis.changed = true;
							break;
						case 102:
							MPEPluszAxis.LSB = data2;
							MPEPluszAxis.changed = true;
							break;
					}
				} else { // Non MPE+ data
					switch (data1) {
						case 0x01: // mod
							mod.val = data2;
							mod.changed = true;
							// std::cout << "mod" << std::endl;
							break;
						case 0x4a: // CC 74 <- we should probably make this assignable if neeed.
							Yaxis.val = data2;
							Yaxis.changed = true;
							break ;
					}
				} // End MPE or MPE+ switch
				if (data1== 0x40 ) {
					pedal = (data2 >= 64);
					if (!pedal) {
						releaseNote(-1);
					}
				} // sustain
				break;
			case 0xe: // pitch wheel, we combine two 7 bit in two bytes into a 14bit msg
				{
				int nBytes;
				// double stamp;
				nBytes = msg.size();
				// for ( i=0; i<nBytes; i++ )
				//     std::cout << "Byte " << i << " = " << (int)msg[i] << ", ";
				// if ( nBytes > 0 )
				//     std::cout << "stamp = " << stamp << std::endl;
		
				// We want 2 bytes but variable size may change with platform, maybe we should do a more robust way
				uint16_t twoBytes ; // Initialize our final pitchWheel variable.
				// we don't need to shift the first byte because it's 7 bit (always starts with 0)
				twoBytes = ( (uint16_t)msg[2] << 7) | ( (uint16_t)msg[1] ) ;
				// std::cout << sizeof(int) << std::endl;
				// std::bitset<8> msgOne(msg[1]);
				// std::bitset<8> msgTwo(msg[2]);
				// std::bitset<16> x(twoBytes);
				//std::cout << "msg[1] and 2 are " << msgOne << " " << msgTwo << " and shifted is " << x << std::endl;
				//std::cout << "twoBytes is " << (int)twoBytes << std::endl;
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

	// std::cout <<" midi input is on " << Global channel!"
	if (this->globalMIDIChannel == (channel + 1) ) {
		std::cout <<"Global channel!" << std::endl;
		if (data1 == midiPedalOne.cc) {
			std::cout <<"Pedal One value is " << data2 << std::endl;
			midiPedalOne.val = data2;
			midiPedalOne.changed = true;
		}
	}
*/
}



