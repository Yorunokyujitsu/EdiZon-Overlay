/**
 * Copyright (C) 2019 - 2020 WerWolv
 * 
 * This file is part of EdiZon
 * 
 * EdiZon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * EdiZon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with EdiZon.  If not, see <http://www.gnu.org/licenses/>.
 */

#define TESLA_INIT_IMPL
#include <tesla.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <switch.h>

#include <switch/nro.h>
#include <switch/nacp.h>

#include "utils.hpp"
#include "cheat.hpp"

#include <unistd.h>
#include <netinet/in.h>

class GuiCheats;

class GuiStats;

class GuiMain : public tsl::Gui {
public:
    GuiMain() { }

    ~GuiMain() { }

    virtual tsl::elm::Element* createUI() {
        auto *rootFrame = new tsl::elm::HeaderOverlayFrame();
        rootFrame->setHeader(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("EdiZon", false, 20, 50+2, 32, renderer->a(tsl::defaultOverlayColor));
            renderer->drawString(APP_VERSION"-ASAP", false, 20, 50+23, 15, renderer->a(tsl::versionTextColor));

            if (edz::cheat::CheatManager::getProcessID() != 0) {
                renderer->drawString("타이틀 ID:", false, 160 +14, 40 -6, 15, renderer->a(tsl::sectitleTextColor));
                renderer->drawString("빌드 ID:", false, 160 +14, 60 -6, 15, renderer->a(tsl::sectitleTextColor));
                renderer->drawString("프로그램 ID:", false, 160 +14, 80 -6, 15, renderer->a(tsl::sectitleTextColor));
                renderer->drawString(GuiMain::s_runningTitleIDString.c_str(), false, 250 +14, 40 -6, 15, renderer->a(tsl::accentTextColor));
                renderer->drawString(GuiMain::s_runningBuildIDString.c_str(), false, 250 +14, 60 -6, 15, renderer->a(tsl::accentTextColor));
                renderer->drawString(GuiMain::s_runningProcessIDString.c_str(), false, 250 +14, 80 -6, 15, renderer->a(tsl::accentTextColor));
            }
        }));

        auto list = new tsl::elm::List();

        if(edz::cheat::CheatManager::isCheatServiceAvailable()){
            auto cheatsItem = new tsl::elm::ListItem("치트 매니저");
            cheatsItem->setClickListener([](s64 keys) {
                if (keys & KEY_A) {
                    tsl::changeTo<GuiCheats>("");
                    return true;
                }
                return false;
            });
            list->addItem(cheatsItem);
        } else {
            auto noDmntSvc = new tsl::elm::ListItem("치트 사용 불가!");
            list->addItem(noDmntSvc);
        }

        auto statsItem  = new tsl::elm::ListItem("시스템 정보");
        statsItem->setClickListener([](s64 keys) {
            if (keys & KEY_A) {
                tsl::changeTo<GuiStats>();
                return true;
            }
            return false;
        });
        list->addItem(statsItem);

        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual void update() { }

public:
    static inline std::string s_runningTitleIDString;
    static inline std::string s_runningProcessIDString;
    static inline std::string s_runningBuildIDString;
    static inline bool b_firstRun = true;
};


class GuiCheats : public tsl::Gui {
public:
    GuiCheats(std::string section) { 
        this->m_section = section;
    }
    ~GuiCheats() { }

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::HeaderOverlayFrame(97);

        rootFrame->setHeader(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("EdiZon", false, 20, 50+2, 32, renderer->a(tsl::defaultOverlayColor));
            renderer->drawString(APP_VERSION"-ASAP", false, 20, 50+23, 15, renderer->a(tsl::versionTextColor));

            if (edz::cheat::CheatManager::getProcessID() != 0) {
                renderer->drawString("타이틀 ID:", false, 160 +14, 40 -6, 15, renderer->a(tsl::sectitleTextColor));
                renderer->drawString("빌드 ID:", false, 160 +14, 60 -6, 15, renderer->a(tsl::sectitleTextColor));
                renderer->drawString("프로그램 ID:", false, 160 +14, 80 -6, 15, renderer->a(tsl::sectitleTextColor));
                renderer->drawString(GuiMain::s_runningTitleIDString.c_str(), false, 250 +14, 40 -6, 15, renderer->a(tsl::accentTextColor));
                renderer->drawString(GuiMain::s_runningBuildIDString.c_str(), false, 250 +14, 60 -6, 15, renderer->a(tsl::accentTextColor));
                renderer->drawString(GuiMain::s_runningProcessIDString.c_str(), false, 250 +14, 80 -6, 15, renderer->a(tsl::accentTextColor));
            }
        }));

        if (edz::cheat::CheatManager::getCheats().size() == 0) {
            auto warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){
                renderer->drawString("\uE150", false, 180, 250, 90, renderer->a(tsl::defaultTextColor));
                renderer->drawString("    치트 로드 실패!", false, 110, 340, 25, renderer->a(tsl::warningTextColor));
            });

            rootFrame->setContent(warning);

        } else {
            auto list = new tsl::elm::List();
            std::string head = "섹션: " + this->m_section;

            if(m_section.length() > 0) list->addItem(new tsl::elm::CategoryHeader(head));
            else list->addItem(new tsl::elm::CategoryHeader("이용 가능한 치트"));

            bool skip = false, inSection = false, submenus = true;
            std::string skipUntil = "";

            for (auto &cheat : edz::cheat::CheatManager::getCheats()) {
                if(cheat->getID() == 1 && cheat->getName().find("--DisableSubmenus--") != std::string::npos)
                    submenus = false;

                if(submenus){
                    // Find section start and end
                    if(this->m_section.length() > 0 && !inSection && cheat->getName().find("--SectionStart:" + this->m_section + "--") == std::string::npos) continue;
                    else if(cheat->getName().find("--SectionStart:" + this->m_section + "--") != std::string::npos) { inSection = true; continue; }
                    else if(inSection && cheat->getName().find("--SectionEnd:" + this->m_section + "--") != std::string::npos) break;

                    // new section
                    if(!skip && cheat->getName().find("--SectionStart:") != std::string::npos){

                        //remove formatting
                        std::string name = cheat->getName();
                        replaceAll(name, "--", "");
                        replaceAll(name, "SectionStart:", "");

                        //create submenu button
                        auto cheatsSubmenu = new tsl::elm::ListItem(name);
                        cheatsSubmenu->setClickListener([name = name](s64 keys) {
                            if (keys & KEY_A) {
                                tsl::changeTo<GuiCheats>(name);
                                return true;
                            }
                            return false;
                        });
                        list->addItem(cheatsSubmenu);
                        this->m_numCheats++;

                        //skip over items in section
                        skip = true;
                        skipUntil = "--SectionEnd:" + name + "--";
                    }
                    // found end of child section
                    else if (skip && cheat->getName().compare(skipUntil) == 0){
                        skip = false;
                        skipUntil = "";
                    }
                    // items to add to section
                    else if(!skip && (inSection || this->m_section.length() < 1)) {
                        std::string cheatNameCheck = cheat->getName();
                        replaceAll(cheatNameCheck, ":ENABLED", "");

                        auto cheatToggleItem = new tsl::elm::ToggleListItem(/*formatString("%d:%s: %s", cheat->getID(), (cheat->isEnabled() ? "y" : "n"),*/ cheatNameCheck/*.c_str()).c_str()*/, cheat->isEnabled());
                        cheatToggleItem->setStateChangedListener([&cheat](bool state) { cheat->setState(state); });

                        this->m_cheatToggleItems.insert({cheat->getID(), cheatToggleItem});
                        list->addItem(cheatToggleItem);
                        this->m_numCheats++;
                    }
                } else {
                    if(cheat->getName().find("--SectionStart:") != std::string::npos || cheat->getName().find("--SectionEnd:") != std::string::npos || cheat->getName().find("--DisableSubmenus--") != std::string::npos)
                        continue;

                    std::string cheatNameCheck = cheat->getName();
                    replaceAll(cheatNameCheck, ":ENABLED", "");

                    auto cheatToggleItem = new tsl::elm::ToggleListItem(cheatNameCheck, cheat->isEnabled());
                    cheatToggleItem->setStateChangedListener([&cheat](bool state) { cheat->setState(state); });

                    this->m_cheatToggleItems.insert({cheat->getID(), cheatToggleItem});
                    list->addItem(cheatToggleItem);
                    this->m_numCheats++;
                }
            }

            // display if no cheats in submenu
            if(this->m_numCheats < 1){
                auto warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){
                    renderer->drawString("\uE150", false, 180, 250, 90, renderer->a(0xFFFF));
                    renderer->drawString("서브 메뉴에 치트가 없습니다!", false, 110, 340, 25, renderer->a(0xFFFF));
                });

                rootFrame->setContent(warning);
            } else rootFrame->setContent(list);
        }

        return rootFrame;
    }

    void replaceAll(std::string& str, const std::string& from, const std::string& to) {
        if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    virtual void update() override {
        for (auto const& [cheatId, toggleElem] : this->m_cheatToggleItems)
            for(auto &cheat : edz::cheat::CheatManager::getCheats())
                if(cheat->getID() == cheatId)
                    toggleElem->setState(cheat->isEnabled());
    }

private:
    int m_numCheats = 0;
    std::string m_section;
    std::map<u32, tsl::elm::ToggleListItem*> m_cheatToggleItems;
};

class GuiStats : public tsl::Gui {
public:
    GuiStats() { 
        if (hosversionAtLeast(8,0,0)) {
            clkrstOpenSession(&this->m_clkrstSessionCpu, PcvModuleId_CpuBus, 3);
            clkrstOpenSession(&this->m_clkrstSessionGpu, PcvModuleId_GPU, 3);
            clkrstOpenSession(&this->m_clkrstSessionMem, PcvModuleId_EMC, 3);
        }

        tsl::hlp::doWithSmSession([this]{
            nifmGetCurrentIpAddress(&this->m_ipAddress);
            this->m_ipAddressString = formatString("%d.%d.%d.%d", this->m_ipAddress & 0xFF, (this->m_ipAddress >> 8) & 0xFF, (this->m_ipAddress >> 16) & 0xFF, (this->m_ipAddress >> 24) & 0xFF);
        });

    }
    ~GuiStats() {
        if (hosversionAtLeast(8,0,0)) {
            clkrstCloseSession(&this->m_clkrstSessionCpu);
            clkrstCloseSession(&this->m_clkrstSessionGpu);
            clkrstCloseSession(&this->m_clkrstSessionMem);
        }
     }

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::OverlayFrame("EdiZon - 시스템 정보", APP_VERSION"-ASAP");


        auto infos = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){

            renderer->drawString("CPU 온도:", false, 45, 160, 18, renderer->a(tsl::defaultTextColor));
            renderer->drawString("PCB 온도:", false, 45, 190, 18, renderer->a(tsl::defaultTextColor));

            renderer->drawRect(x, 203, w, 1, renderer->a(tsl::style::color::ColorFrame));
            renderer->drawString("CPU 클럭:", false, 45, 230, 18, renderer->a(tsl::defaultTextColor));
            renderer->drawString("GPU 클럭:", false, 45, 260, 18, renderer->a(tsl::defaultTextColor));
            renderer->drawString("RAM 클럭:", false, 45, 290, 18, renderer->a(tsl::defaultTextColor));

            renderer->drawRect(x, 303, w, 1, renderer->a(tsl::style::color::ColorFrame));
            renderer->drawString("로컬 IP:", false, 45, 330, 18, renderer->a(tsl::defaultTextColor));


            // Draw temperatures and battery percentage
            static char PCB_temperatureStr[10];
            static char SOC_temperatureStr[10];
            

            ult::ReadSocTemperature(&ult::SOC_temperature, false);
            ult::ReadPcbTemperature(&ult::PCB_temperature, false);

            snprintf(SOC_temperatureStr, sizeof(SOC_temperatureStr) - 1, "%.1f °C", static_cast<double>(ult::SOC_temperature));
            snprintf(PCB_temperatureStr, sizeof(PCB_temperatureStr) - 1, "%.1f °C", static_cast<double>(ult::PCB_temperature));
            

            renderer->drawString(SOC_temperatureStr, false, 240, 160, 18, renderer->a(tsl::accentTextColor));
            renderer->drawString(PCB_temperatureStr, false, 240, 190, 18, renderer->a(tsl::accentTextColor));
            

            u32 cpuClock = 0, gpuClock = 0, memClock = 0;

            if (hosversionAtLeast(8,0,0)) {
                clkrstGetClockRate(&this->m_clkrstSessionCpu, &cpuClock);
                clkrstGetClockRate(&this->m_clkrstSessionGpu, &gpuClock);
                clkrstGetClockRate(&this->m_clkrstSessionMem, &memClock);
            } else {
                pcvGetClockRate(PcvModule_CpuBus, &cpuClock);
                pcvGetClockRate(PcvModule_GPU, &gpuClock);
                pcvGetClockRate(PcvModule_EMC, &memClock);
            }

            renderer->drawString(formatString("%.01f MHz", cpuClock / 1'000'000.0F).c_str(), false, 240, 230, 18, renderer->a(tsl::accentTextColor));
            renderer->drawString(formatString("%.01f MHz", gpuClock / 1'000'000.0F).c_str(), false, 240, 260, 18, renderer->a(tsl::accentTextColor));
            renderer->drawString(formatString("%.01f MHz", memClock / 1'000'000.0F).c_str(), false, 240, 290, 18, renderer->a(tsl::accentTextColor));

            if (this->m_ipAddressString ==  "0.0.0.0")
                renderer->drawString("오프라인", false, 240, 330, 18, renderer->a(tsl::accentTextColor));
            else 
                renderer->drawString(this->m_ipAddressString.c_str(), false, 240, 330, 18, renderer->a(tsl::accentTextColor));

            if(hosversionAtLeast(15,0,0)){
                NifmInternetConnectionType conType;
                u32 wifiStrength;
                NifmInternetConnectionStatus conStatus;
                nifmGetInternetConnectionStatus(&conType, &wifiStrength, &conStatus);
                renderer->drawString("연결 상태:", false, 45, 360, 18, renderer->a(tsl::defaultTextColor));
                if(conStatus == NifmInternetConnectionStatus_Connected && conType == NifmInternetConnectionType_WiFi) {
                    std::string wifiStrengthStr = "(강함)";
                    tsl::Color color = tsl::Color(0x0, 0xF, 0x0, 0xF);
                    if(wifiStrength == 2){
                        wifiStrengthStr = "(보통)";
                        color = tsl::Color(0xE, 0xE, 0x2, 0xF);
                    } else if(wifiStrength <= 1){
                        wifiStrengthStr = "(약함)";
                        color = tsl::Color(0xF, 0x0, 0x0, 0xF);
                    }
                    renderer->drawString("WiFi", false, 240, 360, 18, renderer->a(tsl::accentTextColor));
                    renderer->drawString(wifiStrengthStr.c_str(), false, 285, 360, 18, renderer->a(color));
                } else if(conStatus == NifmInternetConnectionStatus_Connected && conType == NifmInternetConnectionType_Ethernet){
                    renderer->drawString("이더넷", false, 240, 360, 18, renderer->a(tsl::accentTextColor));
                } else {
                    renderer->drawString("연결 없음", false, 240, 360, 18, renderer->a(tsl::accentTextColor));
                }
            } else {
                s32 signalStrength = 0;
                wlaninfGetRSSI(&signalStrength);

                renderer->drawString("WiFi 신호:", false, 45, 360, 18, renderer->a(tsl::defaultTextColor));
                renderer->drawString(formatString("%d dBm", signalStrength).c_str(), false, 240, 360, 18, renderer->a(tsl::accentTextColor)); 
            }
        });
        rootFrame->setContent(infos);

        return rootFrame;
    }

    virtual void update() { }

private:
    ClkrstSession m_clkrstSessionCpu, m_clkrstSessionGpu, m_clkrstSessionMem;
    u32 m_ipAddress;
    std::string m_ipAddressString;
};




class EdiZonOverlay : public tsl::Overlay {
public:
    EdiZonOverlay() { }
    ~EdiZonOverlay() { }

    void initServices() override {
        // GDB Check & Saved Cheat Enabling
        if(edz::cheat::CheatManager::isCheatServiceAvailable()){
            edz::cheat::CheatManager::initialize();
            for (auto &cheat : edz::cheat::CheatManager::getCheats()) {
                if(cheat->getName().find(":ENABLED") != std::string::npos){
                    cheat->setState(true);
                }
            }
        }
        clkrstInitialize();
        pcvInitialize();
        
        i2cInitialize();
        nifmInitialize(NifmServiceType_User);
    } 

    virtual void exitServices() override {
        if (edz::cheat::CheatManager::isCheatServiceAvailable())
            edz::cheat::CheatManager::exit();
        nifmExit();
        i2cExit();
        wlaninfExit();
        nifmExit();
        clkrstExit();
        pcvExit();

    }

    virtual void onShow() override {
        edz::cheat::CheatManager::reload();
        GuiMain::s_runningTitleIDString     = formatString("%016lX", edz::cheat::CheatManager::getTitleID());
        GuiMain::s_runningBuildIDString     = formatString("%016lX", edz::cheat::CheatManager::getBuildID());
        GuiMain::s_runningProcessIDString   = formatString("%lu", edz::cheat::CheatManager::getProcessID());
    }

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<GuiMain>();
    }

    
};


int main(int argc, char **argv) {
    return tsl::loop<EdiZonOverlay>(argc, argv);
}
