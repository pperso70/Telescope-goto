#ifndef IRCONTROLLER_H
#define IRCONTROLLER_H

#include <DIYables_IRcontroller.h>

class IRController
{
private:
    DIYables_IRcontroller_21 irController;
    // String lastCommand;
    String lastCommand = "";
    String currentCommand = "";

    unsigned long lastReceiveTime = 0;
    unsigned long lastRepeatTime = 0;

    const unsigned long repeatDelay = 400; // délai avant répétition
    const unsigned long repeatRate = 150;  // vitesse répétition
    unsigned long pressStartTime = 0;
    unsigned long lastSignalTime;

public:
    IRController(int pin, int debounceTime = 200);
    void begin();
    void update();
    String getCommand();
    bool hasNewCommand();
    void clearCommand();
};

#endif