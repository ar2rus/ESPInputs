#ifndef inputs_h
#define inputs_h

#include "Arduino.h"
#include <iostream>
#include <unordered_map>
using namespace std;

#include <functional>

enum InputState{
	STATE_LOW = LOW,    //0
	STATE_HIGH = HIGH,  //1
	STATE_CHANGE,
};

typedef std::function<void(uint8_t state)> InputHandlerFunction;

class InputEventListener{
	private:
		uint8_t port;
		InputState state;
		uint32_t duration;
		
		InputHandlerFunction handler;
		
		uint8_t c_state;
		uint32_t s_time;
		
	public:
		InputEventListener(uint8_t port, InputState state, uint32_t duration, InputHandlerFunction handler){
			this->port = port;
			this->state = state;
			this->duration = duration;
			this->handler = handler;
			
			pinMode(port, INPUT);
			this->c_state = digitalRead(port);
			this->s_time = 0;
		};
		
		void handle(){
			uint8_t new_state = digitalRead(port);
			if (c_state != new_state){
				c_state = new_state;
				s_time = millis();
			}
			
			if (s_time && (c_state == state || state == STATE_CHANGE) && (millis() - s_time >= duration)){
				s_time = 0;
				if (handler){
					handler(c_state);
				}
			}
		}
		
};

class Inputs{
	private:
		uint16_t index;
		unordered_map<uint16_t, InputEventListener*> tmap;
	public:
		Inputs(){ index = 0; };
		virtual ~Inputs(){
			for (auto it : tmap) {
				free((InputEventListener *)it.second);
			}
		};
    
		uint32_t on(uint8_t port, InputState state, uint32_t duration, InputHandlerFunction handler){
			if (handler){
				tmap[++index] = new InputEventListener(port, state, duration, handler);
				return index;
			}
			return 0;
		};
		
		bool remove(uint16_t index){
			unordered_map<uint16_t, InputEventListener*>::iterator it = tmap.find(index);
			if (it != tmap.end()){
				free(it->second);
			}
			return tmap.erase(index);
		};
		
		void handle(){
			for (auto it : tmap) {
				InputEventListener *t = (InputEventListener *)it.second;
				t->handle();
			}
		}
};

#endif
