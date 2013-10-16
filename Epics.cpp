#include <iostream>
#include <iomanip>
#include "Epics.h"
#include "ConfigManager.h"

using namespace std;

void Epics::exceptionCallback( exception_handler_args args ) {
  string pvname = ( args.chid ? ca_name( args.chid ) : "unknown" );
  cerr << "CA Exception: " << endl
       << "Status: " << ca_message(args.stat) << endl
       << "Channel: " + pvname << endl;
}

void Epics::connectionCallback( connection_handler_args args ) { 
  ConfigManager::I().MutexLock();  
  if ( args.op == CA_OP_CONN_UP ) {
    // channel has connected
    PV* puser = (PV*)ca_puser( args.chid );
    puser->cb(Connected, 0, 0);
   } 
  else { // args.op == CA_OP_CONN_DOWN
    PV* puser = (PV*)ca_puser( args.chid );
    puser->cb(Disconnected, 0, 0);
  }
  ConfigManager::I().MutexUnlock();
}

void Epics::eventCallback( event_handler_args args ) {
  ConfigManager::I().MutexLock();
  if ( args.status != ECA_NORMAL ) {
      cerr << "Error in EPICS event callback" << endl;
  } else {
    dbr_time_double* dbr = (dbr_time_double*)args.dbr; // Convert void* to correct data type
    PV* puser = (PV*)ca_puser( args.chid );             // get pointer to corresponding PV struct
    timespec stamp;
    epicsTimeToTimespec(&stamp, &dbr->stamp);
    double t =  (stamp.tv_sec + stamp.tv_nsec/1.0e9) - I().t0;
    (puser->cb)(NewValue, t, dbr->value);
  }
  ConfigManager::I().MutexUnlock();
}

void Epics::addPV(const string &pvname, Epics::EpicsCallback cb)
{
    ConfigManager::I().MutexLock();
    PV* puser = new PV;
    puser->cb = cb;
    int ca_rtn = ca_create_channel( pvname.c_str(),      // PV name
                                    connectionCallback,  // name of connection callback function
                                    puser,               // 
                                    CA_PRIORITY_DEFAULT, // CA Priority
                                    &puser->mychid );    // Unique channel id
    
    SEVCHK(ca_rtn, "ca_create_channel failed");
    
    ca_rtn = ca_create_subscription( DBR_TIME_DOUBLE,          // CA data type
                                     1,                        // number of elements
                                     puser->mychid,            // unique channel id
                                     DBE_VALUE | DBE_ALARM,    // event mask (change of value and alarm)
                                     eventCallback,            // name of event callback function
                                     puser,
                                     &puser->myevid );         // unique event id needed to clear subscription
      
    SEVCHK(ca_rtn, "ca_create_subscription failed");
    
    pvs[pvname] = puser;
    
    // dont know if this is really meaningful here (also done in ctor)
    ca_poll();        
    ConfigManager::I().MutexUnlock();
    cout << "PV registered" << endl;
}

void Epics::removePV(const string &name)
{
    ConfigManager::I().MutexLock();
    PV* pv = pvs[name];
    ca_clear_subscription ( pv->myevid );
    ca_clear_channel( pv->mychid );
    pvs.erase(name);
    delete pv;    
    ConfigManager::I().MutexUnlock();
    cout << "PV unregisterd" << endl;
    
}

Epics::Epics () {
    ca_context_create( ca_enable_preemptive_callback );
    ca_add_exception_event( exceptionCallback, NULL );
    ca_poll();
    timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t0 = t.tv_sec + t.tv_nsec/1.0e9;
}

Epics::~Epics () {
    for (map<string, PV*>::iterator it = pvs.begin(); it != pvs.end(); ++it ) {
        removePV(it->first);
    }
    pvs.clear();
    ca_context_destroy();
    cout << "EPICS dtor" << endl;
}
