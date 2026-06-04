#include "IRController.h"

IRController::IRController(int pin, int debounceTime) : irController(pin, debounceTime)
{
}

void IRController::begin()
{
    irController.begin();
}

String IRController::getCommand()
{
    return lastCommand;
}

bool IRController::hasNewCommand()
{
    return lastCommand != "";
}

void IRController::clearCommand()
{
    lastCommand = "";
}

void IRController::update()
{
    unsigned long now = millis();

    Key21 command = irController.getKey();

    if (command != Key21::NONE)
    {

        lastSignalTime = now; // IMPORTANT

        String cmd;

        switch (command)
        {
        case Key21::KEY_0:
            cmd = "0";
            break;
        case Key21::KEY_1:
            cmd = "1";
            break;
        case Key21::KEY_2:
            cmd = "2";
            break;
        case Key21::KEY_3:
            cmd = "3";
            break;
        case Key21::KEY_4:
            cmd = "4";
            break;
        case Key21::KEY_5:
            cmd = "5";
            break;
        case Key21::KEY_6:
            cmd = "6";
            break;
        case Key21::KEY_7:
            cmd = "7";
            break;
        case Key21::KEY_8:
            cmd = "8";
            break;
        case Key21::KEY_9:
            cmd = "9";
            break;
        case Key21::KEY_CH_MINUS:
            cmd = "CH-";
            break;
        case Key21::KEY_CH:
            cmd = "CH";
            break;
        case Key21::KEY_CH_PLUS:
            cmd = "CH+";
            break;
        case Key21::KEY_PREV:
            cmd = "<<";
            break;
        case Key21::KEY_NEXT:
            cmd = ">>";
            break;
        case Key21::KEY_PLAY_PAUSE:
            cmd = ">|";
            break;
        case Key21::KEY_VOL_MINUS:
            cmd = "-";
            break;
        case Key21::KEY_VOL_PLUS:
            cmd = "+";
            break;
        case Key21::KEY_EQ:
            cmd = "EQ";
            break;
        case Key21::KEY_100_PLUS:
            cmd = "100";
            break;
        case Key21::KEY_200_PLUS:
            cmd = "200";
            break;
        default:
            return;
        }
      
        // NOUVEL APPUI
        if (cmd != currentCommand)
        {
            currentCommand = cmd;
            pressStartTime = now;
            lastRepeatTime = now;
            lastCommand = cmd; // envoi immédiat
            return;
        }

        // Si on ne reçoit plus rien depuis 150ms → relâchement
        if (currentCommand != "" && (now - lastSignalTime > 150))
        {
            currentCommand = "";
            return;
        }

        // Gestion repeat indépendante de la réception IR
        if (currentCommand != "")
        {
            if (now - pressStartTime >= 300 &&
                now - lastRepeatTime >= 200)
            {

                lastRepeatTime = now;
                lastCommand = currentCommand;
            }
        }
    }
}