#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "Epics.h"
#include "Window.h"
#include "SimpleGraph.h"
#include "TextLabel.h"
#include "StopWatch.h"

class PlotWindow: public Window {
private:
    std::string _pvname; // the EPICS PV name
    std::string _xlabel;
    std::string _ylabel;
    
    bool _initialized;
  
    UnitBorderBox WindowArea;
    SimpleGraph graph;
    TextLabel text;

    int frame;          //for debug
    
    void ProcessEpicsData(const Epics::DataItem *i);
    void ProcessEpicsProperties(const std::string &attr, void *d);
    std::string callbackSetBackLength(const std::string& arg);

    bool _epics_connected;

    TextLabel discon_lbl;
    
public:
    const std::string& Xlabel() const { return _xlabel; }
    const std::string& Ylabel() const { return _ylabel; }
    std::string& Xlabel() { return _xlabel; }
    std::string& Ylabel() { return _ylabel; }

    PlotWindow( 
            WindowManager* owner,
            const std::string& pvname, 
            const std::string& xlabel = "Always label your axes",
            const std::string& ylabel = "Always label your axes",
            const float xscale = 1,
            const float yscale = 1);

    virtual ~PlotWindow();


    virtual void Update();
    virtual void Draw();
    virtual int Init();

};

std::ostream& operator<<( std::ostream& stream, const PlotWindow& win );

#endif // PLOTWINDOW_H
