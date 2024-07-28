/*
    Output Interface
    Copyright (C) 2024 Jasem Mutlaq (mutlaqja@ikarustech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include "indioutputinterface.h"
#include <cstring>
#include "indilogger.h"

namespace INDI
{

/////////////////////////////////////////////////////////////////////////////////////////////
///
/////////////////////////////////////////////////////////////////////////////////////////////
OutputInterface::OutputInterface(DefaultDevice *defaultDevice) : m_defaultDevice(defaultDevice)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/////////////////////////////////////////////////////////////////////////////////////////////
OutputInterface::~OutputInterface()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/////////////////////////////////////////////////////////////////////////////////////////////
void OutputInterface::initProperties(const char *groupName, uint8_t Outputs, const std::string &prefix)
{
    DigitalOutputLabelsTP.reserve(Outputs);

    // Initialize labels
    for (auto i = 0; i < Outputs; i++)
    {
        auto name = "DIGITAL_OUTPUT_" + std::to_string(i + 1);
        auto label = prefix + " #" + std::to_string(i + 1);

        INDI::WidgetText oneLabel;
        oneLabel.fill(name, label, label);
        DigitalOutputLabelsTP.push(std::move(oneLabel));
    }

    DigitalOutputLabelsTP.fill(m_defaultDevice->getDeviceName(), "DIGITAL_OUTPUT_LABELS", "Labels", groupName, IP_RW, 60,
                               IPS_IDLE);
    DigitalOutputLabelsTP.shrink_to_fit();
    DigitalOutputLabelsTP.load();

    DigitalOutputsSP.reserve(Outputs);
    // Initialize switches, use labels if loaded.
    for (size_t i = 0; i < Outputs; i++)
    {
        auto name = "DIGITAL_OUTPUT_" + std::to_string(i + 1);
        auto label = prefix + " #" + std::to_string(i + 1);

        INDI::PropertySwitch oneOutput {2};
        oneOutput[Off].fill("OFF", "Off", ISS_OFF);
        oneOutput[On].fill("ON", "On", ISS_OFF);

        if (i < DigitalOutputLabelsTP.count())
            label = DigitalOutputLabelsTP[i].getText();
        oneOutput.fill(m_defaultDevice->getDeviceName(), name.c_str(), label.c_str(), groupName, IP_RW, ISR_ATMOST1, 60, IPS_IDLE);
        DigitalOutputsSP.push_back(std::move(oneOutput));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/////////////////////////////////////////////////////////////////////////////////////////////
bool OutputInterface::updateProperties()
{
    if (m_defaultDevice->isConnected())
    {
        for (auto &oneOutput : DigitalOutputsSP)
            m_defaultDevice->defineProperty(oneOutput);
        m_defaultDevice->defineProperty(DigitalOutputLabelsTP);
    }
    else
    {
        for (auto &oneOutput : DigitalOutputsSP)
            m_defaultDevice->deleteProperty(oneOutput);
        m_defaultDevice->deleteProperty(DigitalOutputLabelsTP);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/////////////////////////////////////////////////////////////////////////////////////////////
bool OutputInterface::processSwitch(const char *dev, const char *name, ISState states[], char *names[], int n)
{
    if (dev && !strcmp(dev, m_defaultDevice->getDeviceName()))
    {
        for (size_t i = 0; i < DigitalOutputsSP.size(); i++)
        {
            if (DigitalOutputsSP[i].isNameMatch(name))
            {
                auto oldState = DigitalOutputsSP[i].findOnSwitchIndex();
                DigitalOutputsSP[i].update(states, names, n);
                auto newState = DigitalOutputsSP[i].findOnSwitchIndex();
                if (oldState != newState)
                {
                    // Cast to Command and send
                    if (CommandOutput(i, static_cast<OutputState>(newState)))
                    {
                        DigitalOutputsSP[i].setState(IPS_OK);
                    }
                    else
                    {
                        DigitalOutputsSP[i].setState(IPS_ALERT);
                        DigitalOutputsSP[i].reset();
                        DigitalOutputsSP[i][oldState].setState(ISS_ON);
                    }

                    // Apply and return
                    DigitalOutputsSP[i].apply();
                    return true;
                }
                // No state change
                else
                {
                    DigitalOutputsSP[i].setState(IPS_OK);
                    DigitalOutputsSP[i].apply();
                    return true;
                }
            }

        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/////////////////////////////////////////////////////////////////////////////////////////////
bool OutputInterface::processText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev && !strcmp(dev, m_defaultDevice->getDeviceName()))
    {
        // If this call due to config loading, let's delete existing dummy property and define the full one
        if (DigitalOutputLabelsTP.isNameMatch(name))
        {
            DigitalOutputLabelsTP.update(texts, names, n);
            DigitalOutputLabelsTP.setState(IPS_OK);
            DigitalOutputLabelsTP.apply();
            m_defaultDevice->saveConfig(DigitalOutputLabelsTP);
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/////////////////////////////////////////////////////////////////////////////////////////////
bool OutputInterface::saveConfigItems(FILE *fp)
{
    DigitalOutputLabelsTP.save(fp);
    return true;
}

}