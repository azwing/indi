/*******************************************************************************
  created 2014 G. Schmidt

  derived from gpusb code from Jasem Mutlaq

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/

#pragma once

#include "defaultdevice.h"
#include "indiguiderinterface.h"

#include <ctime>

class STAR2000 : public INDI::DefaultDevice, public INDI::GuiderInterface
{
    public:
        STAR2000();

        virtual bool initProperties() override;
        virtual bool updateProperties() override;
        virtual void ISGetProperties(const char *dev) override;
        virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
        virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
        virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
        virtual bool ISSnoopDevice(XMLEle *root) override;

    protected:
        virtual bool saveConfigItems(FILE *fp) override;

        //  Generic indi device entries
        bool Connect() override;
        bool Connect(char *);
        bool Disconnect() override;
        const char *getDefaultName() override;

        void TimerHit() override;

        virtual IPState GuideNorth(uint32_t ms) override;
        virtual IPState GuideSouth(uint32_t ms) override;
        virtual IPState GuideEast(uint32_t ms) override;
        virtual IPState GuideWest(uint32_t ms) override;

    private:
        float CalcWEPulseTimeLeft();
        float CalcNSPulseTimeLeft();

    public:
        // STAR2000 box RS232 port
        ITextVectorProperty PortTP;
        IText PortT[1] {};

    private:
        bool InWEPulse { false };
        float WEPulseRequest { 0 };
        struct timeval WEPulseStart
        {
            0, 0
        };
        int WEtimerID { 0 };

        bool InNSPulse { false };
        float NSPulseRequest { 0 };
        struct timeval NSPulseStart
        {
            0, 0
        };
        int NStimerID { 0 };

        int WEDir { 0 };
        int NSDir { 0 };
};
