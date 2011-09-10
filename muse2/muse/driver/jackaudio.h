//=========================================================
//  MusE
//  Linux Music Editor
//    $Id: jackaudio.h,v 1.20.2.4 2009/12/20 05:00:35 terminator356 Exp $
//  (C) Copyright 2002 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//=========================================================

#ifndef __JACKAUDIO_H__
#define __JACKAUDIO_H__

#include <jack/jack.h>
#include "audiodev.h"

class MidiPlayEvent;

namespace MusEGlobal {
bool checkAudioDevice();
}

//---------------------------------------------------------
//   JackAudioDevice
//---------------------------------------------------------

class JackAudioDevice : public AudioDevice {

      jack_client_t* _client;
      double sampleTime;
      int samplePos;
      jack_transport_state_t transportState;
      jack_position_t pos;
      char jackRegisteredName[16];
      int dummyState;
      int dummyPos;
      // Free-running frame counter incremented always in process.
      jack_nframes_t _frameCounter; 
      
      static int processAudio(jack_nframes_t frames, void*);

   public:
      JackAudioDevice(jack_client_t* cl, char * jack_id_string);
      virtual ~JackAudioDevice();
      virtual void nullify_client() { _client = 0; }
      
      virtual inline int deviceType() const { return JACK_AUDIO; }   // p3.3.52
      
      void scanMidiPorts();
      
      //virtual void start();
      virtual void start(int);
      virtual void stop ();
      virtual bool dummySync(int state); // Artificial sync when not using Jack transport.
      
      virtual int framePos() const;
      virtual unsigned frameTime() const     { return _frameCounter; }  

      virtual float* getBuffer(void* port, unsigned long nframes) {
            return (float*)jack_port_get_buffer((jack_port_t*)port, nframes);
            }

      virtual std::list<QString> outputPorts(bool midi = false, int aliases = -1);
      virtual std::list<QString> inputPorts(bool midi = false, int aliases = -1);

      virtual void registerClient();
      virtual const char* clientName() { return jackRegisteredName; }

      //virtual void* registerOutPort(const char* name);
      //virtual void* registerInPort(const char* name);
      virtual void* registerOutPort(const char* /*name*/, bool /*midi*/);
      virtual void* registerInPort(const char* /*name*/, bool /*midi*/);

      //virtual char* getJackName();

      virtual void unregisterPort(void*);
      virtual void connect(void*, void*);
      virtual void disconnect(void*, void*);
      virtual int connections(void* clientPort) { return jack_port_connected((jack_port_t*)clientPort); }
      virtual void setPortName(void* p, const char* n) { jack_port_set_name((jack_port_t*)p, n); }
      virtual void* findPort(const char* name);
      virtual QString portName(void* port);
      virtual int getState();
      virtual unsigned int getCurFrame();
      virtual bool isRealtime()          { return jack_is_realtime(_client); }
      virtual int realtimePriority() const;
      virtual void startTransport();
      virtual void stopTransport();
      virtual void seekTransport(unsigned frame);
      virtual void seekTransport(const Pos &p);
      virtual void setFreewheel(bool f);
      jack_transport_state_t transportQuery(jack_position_t* pos);
      void graphChanged();
      void registrationChanged();
      void connectJackMidiPorts();
      
      virtual int setMaster(bool f);

      //static bool jackStarted;
      };

#endif
