//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: event.cpp,v 1.8.2.5 2009/12/20 05:00:35 terminator356 Exp $
//
//  (C) Copyright 2000-2003 Werner Schweer (ws@seh.de)
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

#include <stdio.h>
#include "event.h"
#include "waveevent.h"
#include "midievent.h"
#include "midi_consts.h"
#include "part.h"

// Forwards from header:
#include "eventbase.h"
#include "part.h"
#include "xml.h"
#include "audio_fifo.h"

namespace MusECore {

EventID_t EventBase::idGen=0;

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

EventBase::EventBase(EventType t)
      {
      _type     = t;
      Pos::setType(_type == Wave ? FRAMES : TICKS);
      refCount  = 0;
      _selected = false;
      _uniqueId = newId();
      _id = _uniqueId;
      }

EventBase::EventBase(const EventBase& ev, bool duplicate_not_clone)
  : PosLen(ev)
      {
      refCount  = 0;
      _selected = ev._selected;
      _type     = ev._type;
      _uniqueId = newId();
      _id = duplicate_not_clone ? _uniqueId : ev._id;
      }
      
void EventBase::assign(const EventBase& ev)
{
  if(this == &ev) // Is it a shared clone?
    return;
  
  if(ev.type() != _type)
    return;
  
  (PosLen&)(*this) = (const PosLen&)ev;
  setSelected(ev.selected());
}

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void EventBase::move(int tickOffset)
      {
      setTick(tick() + tickOffset);
      }

bool EventBase::isSimilarType(const EventBase& e,
            bool compareTime,
            bool compareA, bool compareB, bool compareC,
            bool compareWavePath, bool compareWavePos, bool compareWaveStartPos) const
{
  // Types must be the same.
  if((e.type() != type()) ||
     // Compares base Pos positions.
     (compareTime && *this != e))
    return false;

  switch(type())
  {
    case Note:
      // Are the note, and on and off velocities the same?
      return (!compareA || e.dataA() == dataA()) &&
             (!compareB || e.dataB() == dataB()) &&
             (!compareC || e.dataC() == dataC());
    break;
    
    case Controller:
      // Are the controller numbers and values the same?
      return (!compareA || e.dataA() == dataA()) &&
             (!compareB || e.dataB() == dataB());
    break;
    
    case Sysex:
    case Meta:
      // Are the sysex or meta length and data the same?
      return dataLen() == e.dataLen() && (dataLen() == 0 || (memcmp( constData(), e.constData(), dataLen()) == 0));
    break;
    
    case Wave:
      // Are the sound file path, wave starting position and event position and length the same?
      return (!compareWavePos || this->PosLen::operator==(e)) &&
             (!compareWaveStartPos || spos() == e.spos()) &&
             (!compareWavePath || sndFile().dirPath() == e.sndFile().dirPath());
    break;
  }
  return false;
}

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void EventBase::dump(int n) const
      {
      for (int i = 0; i < n; ++i)
            putchar(' ');
      printf("Event %p refs:%d ", this, refCount);
      PosLen::dump(n+2);
      }

//---------------------------------------------------------
//   duplicate
//---------------------------------------------------------

Event Event::duplicate() const
      {
      return ev ? Event(ev->duplicate()) : Event();
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

Event Event::clone() const
      {
      return ev ? Event(ev->clone()) : Event();
      }

//---------------------------------------------------------
//   deClone
//---------------------------------------------------------

void Event::deClone() 
{ 
  if(ev)
    ev->deClone(); 
} 

Event::Event() 
{ 
  ev = 0; 
}

Event::Event(EventType t) {
            if (t == Wave)
                  ev = new MusECore::WaveEventBase(t);
            else
                  ev = new MusECore::MidiEventBase(t);
            ++(ev->refCount);
            }
Event::Event(const Event& e) {
            ev = e.ev;
            if(ev)
              ++(ev->refCount);
          }
Event::Event(EventBase* eb) {
            ev = eb;
	    if(ev)
              ++(ev->refCount);
            }
            
Event::~Event() {
            if (ev && --(ev->refCount) == 0) {
                  delete ev;
                  ev=0;
                  }
            }

MidiPlayEvent Event::asMidiPlayEvent(unsigned time, int port, int channel) const
      {
      MidiPlayEvent mpe;  
      mpe.setChannel(channel);
      mpe.setTime(time);
      mpe.setPort(port);
      mpe.setLoopNum(0);
      switch(type()) {
            case Note:
                  mpe.setType(ME_NOTEON);
                  mpe.setA(dataA());
                  mpe.setB(dataB());
                  break;
            case Controller:
                  mpe.setType(ME_CONTROLLER);
                  mpe.setA(dataA());  // controller number
                  mpe.setB(dataB());  // controller value
                  break;
            case Sysex:
                  mpe.setType(ME_SYSEX);
                  mpe.setData(eventData());
                  break;
            default:
                  fprintf(stderr, "Event::asMidiPlayEvent: event type %d not implemented\n",
                     type());
                  break;
            }
      return mpe;
      }
            
bool Event::empty() const      { return ev == 0; }
EventType Event::type() const  { return ev ? ev->type() : Note;  }
EventID_t Event::id() const { return ev ? ev->id() : MUSE_INVALID_EVENT_ID; }
void Event::shareId(const Event& e) { if(ev && e.ev) ev->shareId(e.ev); }

void Event::setType(EventType t) {
            if (ev && --(ev->refCount) == 0) {
                  delete ev;
                  ev = 0;
                  }
            if (t == Wave)
                  ev = new MusECore::WaveEventBase(t);
            else
                  ev = new MusECore::MidiEventBase(t);
            ++(ev->refCount);
            }

Event& Event::operator=(const Event& e) {
            if (ev != e.ev)
            {
              if (ev && --(ev->refCount) == 0) {
                    delete ev;
                    ev = 0;
                    }
              ev = e.ev;
              if (ev)
                    ++(ev->refCount);
            }      
            return *this;
            }

void Event::assign(const Event& e)
{
  if(!ev || !e.ev || ev == e.ev)
    return;
  ev->assign(*(e.ev));
}

bool Event::operator==(const Event& e) const {
            return ev == e.ev;
            }

bool Event::isSimilarTo(const Event& other) const
{
		return ev ? ev->isSimilarTo(*other.ev) : (other.ev ? false : true);
}

bool Event::isSimilarType(const Event& e,
            bool compareTime,
            bool compareA, bool compareB, bool compareC,
            bool compareWavePath, bool compareWavePos, bool compareWaveStartPos) const
{
  return ev ? ev->isSimilarType(*e.ev,
         compareTime,
         compareA, compareB, compareC,
         compareWavePath, compareWavePos, compareWaveStartPos) :
       (e.ev ? false : true);
}

int Event::getRefCount() const    { return ev ? ev->getRefCount() : 0; }
bool Event::selected() const      { return ev ? ev->_selected : false; }
void Event::setSelected(bool val) { if(ev) ev->_selected = val; }
void Event::move(int offset)      { if(ev) ev->move(offset); }

void Event::read(Xml& xml)            
{ 
  if(ev) ev->read(xml); 
}


void Event::write(int a, Xml& xml, const Pos& o, bool forceWavePaths) const { if(ev) ev->write(a, xml, o, forceWavePaths); }
void Event::dump(int n) const     { if(ev) ev->dump(n); }
Event Event::mid(unsigned a, unsigned b) const { return ev ? Event(ev->mid(a, b)) : Event(); }

bool Event::isNote() const                   { return ev ? ev->isNote() : false;        }
bool Event::isNoteOff() const                { return ev ? ev->isNoteOff() : false;     }
bool Event::isNoteOff(const Event& e) const  { return ev ? (e.isNoteOff() && (e.pitch() == dataA())) : false; }
int Event::dataA() const                     { return ev ? ev->dataA() : 0;  }
int Event::pitch() const                     { return ev ? ev->dataA() : 0;  }
void Event::setA(int val)                    { if(ev) ev->setA(val);       }
void Event::setPitch(int val)                { if(ev) ev->setA(val);       }
int Event::dataB() const                     { return ev ? ev->dataB() : 0;  }
int Event::velo() const                      { return ev ? ev->dataB() : 0;  }
void Event::setB(int val)                    { if(ev) ev->setB(val);       }
void Event::setVelo(int val)                 { if(ev) ev->setB(val);       }
int Event::dataC() const                     { return ev ? ev->dataC() : 0;  }
int Event::veloOff() const                   { return ev ? ev->dataC() : 0;  }
void Event::setC(int val)                    { if(ev) ev->setC(val);       }
void Event::setVeloOff(int val)              { if(ev) ev->setC(val);       }

const unsigned char* Event::data() const     { return ev ? ev->constData() : 0;    }
int Event::dataLen() const                   { return ev ? ev->dataLen() : 0; }
void Event::setData(const unsigned char* data, int len) { if(ev) ev->setData(data, len); }
const EvData Event::eventData() const        { return ev ? ev->eventData() : EvData(); }

const QString Event::name() const            { return ev ? ev->name() : QString();  }
void Event::setName(const QString& s)        { if(ev) ev->setName(s);     }
int Event::spos() const                      { return ev ? ev->spos() : 0;  }
void Event::setSpos(int s)                   { if(ev) ev->setSpos(s);     }
MusECore::SndFileR Event::sndFile() const    { return ev ? ev->sndFile() : MusECore::SndFileR(); }

void Event::setSndFile(MusECore::SndFileR& sf) 
{ 
  if(ev) ev->setSndFile(sf);   
}

void Event::readAudio(unsigned offset, float** bpp, int channels, int nn, bool doSeek, bool overwrite)
      {
        if(ev) ev->readAudio(offset, bpp, channels, nn, doSeek, overwrite);
      }
void Event::seekAudio(sf_count_t offset)
      {
        if(ev) ev->seekAudio(offset);
      }
Fifo* Event::audioPrefetchFifo()
{
        return ev ? ev->audioPrefetchFifo() : 0;
}
void Event::prefetchAudio(Part* part, sf_count_t frames)
{
        if(ev) ev->prefetchAudio(part, frames);
}
      
//--------------------------------------------------------
// 'Agnostic' position methods - can be TICKS and FRAMES.
//--------------------------------------------------------

Pos Event::pos() const                  { return ev ? *ev : Pos(); }
void Event::setPos(const Pos& p)        { if(ev) ev->setPos(p); }
Pos Event::end() const                  { return ev ? ev->end() : Pos(); }
PosLen Event::posLen() const            { return ev ? *ev : PosLen(); }

unsigned Event::posValue() const        { return ev ? ev->posValue() : 0; }
unsigned Event::posValue(Pos::TType time_type) const { return ev ? ev->posValue(time_type) : 0; }
void Event::setPosValue(unsigned val)   { if(ev) ev->setPosValue(val); }
void Event::setPosValue(unsigned val, Pos::TType time_type) { if(ev) ev->setPosValue(val, time_type); }

unsigned Event::lenValue() const        { return ev ? ev->lenValue() : 0; }
unsigned Event::lenValue(Pos::TType time_type) const { return ev ? ev->lenValue(time_type) : 0; }
void Event::setLenValue(unsigned val)   { if(ev) ev->setLenValue(val); }
void Event::setLenValue(unsigned val, Pos::TType time_type) { if(ev) ev->setLenValue(val, time_type); }

unsigned Event::endPosValue() const     { return ev ? ev->endValue() : 0; }

//--------------------------------------------------------
// 'Resolving' position methods - must be TICKS or FRAMES.
//--------------------------------------------------------

unsigned Event::tick() const            { return ev ? ev->tick() : 0; }
void Event::setTick(unsigned val)       { if(ev) ev->setTick(val); }
unsigned Event::frame() const           { return ev ? ev->frame() : 0; }
void Event::setFrame(unsigned val)      { if(ev) ev->setFrame(val); }
unsigned Event::lenTick() const         { return ev ? ev->lenTick() : 0; }
void Event::setLenTick(unsigned val)    { if(ev) ev->setLenTick(val); }
unsigned Event::lenFrame() const        { return ev ? ev->lenFrame() : 0; }
void Event::setLenFrame(unsigned val)   { if(ev) ev->setLenFrame(val); }
unsigned Event::endTick() const         { return ev ? ev->end().tick() : 0; }
unsigned Event::endFrame() const        { return ev ? ev->end().frame() : 0; }

} // namespace MusECore
