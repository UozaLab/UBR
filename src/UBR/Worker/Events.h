/*
 * Copyright (c) 2024-2025, UozaLab
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __EVENTS__H__
#define __EVENTS__H__

#include <wx/wx.h>

enum ThreadState
{
    THREAD_NOTSTARTED,
    THREAD_STARTED,
    THREAD_FINISHED,
    THREAD_TERMINATING,
    THREAD_TERMINATED,
};

class CustomThread : public wxThread
{
protected:
    volatile ThreadState state;
    wxEvtHandler* event_handler;
    wxCriticalSection cs;
    void send_event()
    {
        wxThreadEvent* ev = new wxThreadEvent();
        ev->SetPayload(const_cast<ThreadState>(state));
        wxQueueEvent(event_handler, ev);
    }

public:
    CustomThread(wxEvtHandler* _event_handler):
      wxThread(wxTHREAD_JOINABLE),
      state(THREAD_NOTSTARTED),
      event_handler(_event_handler)
    {
    }
    void Terminate()
    {
        wxCriticalSectionLocker locker(cs);
        if(state == THREAD_STARTED)
        {
            state = THREAD_TERMINATING;
            send_event();
        }
    }
    bool TerminateRequired()
    {
        wxCriticalSectionLocker locker(cs);
        return state == THREAD_TERMINATING;
    }
    ThreadState SetStateComplete()
    {
        wxCriticalSectionLocker locker(cs);
        state = (state == THREAD_TERMINATING) ? THREAD_TERMINATED:
                (state == THREAD_TERMINATED) ? THREAD_TERMINATED:
                THREAD_FINISHED;
        send_event();
        return state;
    }
    ThreadState SetStateStart()
    {
        wxCriticalSectionLocker locker(cs);
        state = THREAD_STARTED;
        send_event();
        return state;
    }

};

class ProgressEvent;
wxDECLARE_EVENT(myEVT_PROGRESS, ProgressEvent);

class ProgressEvent : public wxEvent
{
protected:
    double ratio;
public:
    ProgressEvent(double _ratio, wxEventType commandEventType = myEVT_PROGRESS, int id = wxEVT_ANY)
         : wxEvent(id, commandEventType), ratio(_ratio)
    {
    }
    ProgressEvent(const ProgressEvent &event)
      : wxEvent(event)
    {
        this->ratio = event.ratio;
    }
    ~ProgressEvent()
    {
    }
    double GetData()
    {
        return ratio;
    }
    
    virtual wxEvent* Clone() const wxOVERRIDE { return new ProgressEvent(*this); }

};

class MsgEvent;
wxDECLARE_EVENT(myEVT_MSG, MsgEvent);

class MsgEvent : public wxEvent
{
protected:
    wxString what;
public:
    MsgEvent(wxString _what, wxEventType commandEventType = myEVT_MSG, int id = wxEVT_ANY)
         : wxEvent(id, commandEventType), what(_what)
    {
    }
    MsgEvent(const MsgEvent &event)
      : wxEvent(event)
    {
        this->what = event.what;
    }
    ~MsgEvent()
    {
    }
    wxString GetData()
    {
        return what;
    }
    
    virtual wxEvent* Clone() const wxOVERRIDE { return new MsgEvent(*this); }

};


class ErrorEvent;
wxDECLARE_EVENT(myEVT_ERROR, ErrorEvent);

class ErrorEvent : public wxEvent
{
protected:
    wxString reason;
public:
    ErrorEvent(wxString _reason, wxEventType commandEventType = myEVT_ERROR, int id = wxEVT_ANY)
         : wxEvent(id, commandEventType), reason(_reason)
    {
    }
    ErrorEvent(const ErrorEvent &event)
      : wxEvent(event)
    {
        this->reason = event.reason;
    }
    ~ErrorEvent()
    {
    }
    wxString GetData()
    {
        return reason;
    }
    
    virtual wxEvent* Clone() const wxOVERRIDE { return new ErrorEvent(*this); }

};



#endif
