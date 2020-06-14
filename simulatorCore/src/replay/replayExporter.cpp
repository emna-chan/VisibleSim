/**
 * @file   replayExporter.cpp
 * @author Pierre Thalamy <pthalamy@pierre-ZenBook-UX433FA>
 * @date   Tue Jun  9 11:14:00 2020
 *
 * @brief Simulation data exporter to be used for simulation replay
 *
 *
 */

#include <algorithm>
#include "replayExporter.h"

#include "replayTags.h"

#include "../utils/utils.h"
#include "../base/simulator.h"

using namespace BaseSimulator;
using namespace ReplayTags;

ReplayExporter::ReplayExporter() {
    const string& fnbin = buildExportFilename();

    cout << debug << " rwwa " << endl;

    cout << TermColor::BWhite
         << "(replay) exporting simulation data to file: " << TermColor::Reset
         << fnbin << endl;

    exportFile = new ofstream(fnbin, ios::out | ios::binary);

    if (debug) {
        const string& fndebug = debugFilenameFromExportFilename(fnbin);

        cout << TermColor::BWhite
             << "(replay) exporting replay debug to file: " << TermColor::Reset
             << fndebug << endl;

        debugFile = new ofstream(fndebug, ios::out);
    }
}

string ReplayExporter::buildExportFilename() const {
    const string& cliFilename = Simulator::getSimulator()->getCmdLine().getReplayFilename();
    if (not cliFilename.empty())
        return cliFilename;

    auto& cmdLine = Simulator::getSimulator()->getCmdLine();

    string appName = cmdLine.getApplicationName();
    // if appname starts with "./" strip it
    if (appName[0] == '.' and appName[1] == '/')
        appName = appName.substr(2, appName.size());

    string configName = cmdLine.getConfigFile();

    // assumes config name format *.xml
    configName = configName.substr(0, configName.size() - 4);

    stringstream prefix;
    prefix << "replay_" << appName;
    if (configName != "config")
        prefix << "_" << configName;

    return utils::generateTimestampedFilename(prefix.str(), extension, true);
}

string ReplayExporter::debugFilenameFromExportFilename(const string& exportFn) const {
    string str = string(exportFn);

    std::size_t ext = str.find_last_of(".");

//    return str.replace(str.begin() + ext, str.end(), ".txt");
    return "debug.txt";
}


void ReplayExporter::endExport() {
    writeKeyFramesIndex();

    if (exportFile) {
        exportFile->close();
        delete exportFile;
        exportFile = nullptr;
    }

    if (debugFile) {
        debugFile->close();
        delete debugFile;
        debugFile = nullptr;
    }
}

bool ReplayExporter::isReplayEnabled() {
    return (Simulator::getSimulator()->getCmdLine().isReplayEnabled() && enabled);
}

void ReplayExporter::writeHeader() {
    exportFile->write((char*)&VS_MAGIC, sizeof(u4));

    const u1 moduleType = BaseSimulator::getWorld()->getBlockType();
    const Cell3DPosition& gridSize = BaseSimulator::getWorld()->lattice->gridSize;

    exportFile->write((char*)&moduleType, sizeof(u1));
    exportFile->write((char*)&gridSize, 3*sizeof(u2)); // xyz

    keyFramesIndexPos = exportFile->tellp();

    // cout << "keyFramesIndexPos: " << keyFramesIndexPos << endl;
    cout << "debug:" << debug << endl;
    if (debug) {
        debugFile->write((char*)&VS_MAGIC, sizeof(u4)); *debugFile << endl;
        *debugFile << (int)moduleType << endl;
        *debugFile << gridSize[0] << " " << gridSize[1] << " " << gridSize[2] << endl; // xyz

        keyFramesIndexPosDebug = debugFile->tellp();
        // cout << "keyFramesIndexPosDebug: " << keyFramesIndexPosDebug << endl;
    }
}

void ReplayExporter::writeKeyFramesIndex() {
    // Move write head to previously saved index location
    /*exportFile->seekp(keyFramesIndexPos); // currently at the end !
    if (debug) debugFile->seekp(keyFramesIndexPosDebug);*/

    // Write number of index entries
    size_t nEntries = keyFramesIndex.size();
    exportFile->write((char*)&nEntries, sizeof(size_t));

    if (debug) {
        *debugFile << "-- BEGIN KEY FRAME INDEX --" << endl;
        *debugFile << nEntries << endl;
    }

    //  and write all key frame pairs (kfp) to file
    for (const auto& kfp : keyFramesIndex) {
        exportFile->write((char*)&kfp.first, sizeof(Time));
        exportFile->write((char*)&kfp.second, sizeof(streampos));

        if (debug) *debugFile << kfp.first << " " << kfp.second << endl;
    }

    if (debug) *debugFile << "-- END KEY FRAME INDEX --" << endl;
}

void ReplayExporter::writeKeyFrameIfNeeded(Time date) {
    if (date > (lastKeyFrameExportDate + keyFrameSaveFrequency)) {
        writeKeyFrame(date);
        lastKeyFrameExportDate = date;
    }
}

void ReplayExporter::writeKeyFrame(Time date) {
    keyFramesIndex.insert(make_pair(date, exportFile->tellp()));

    size_t nbModules = BaseSimulator::getWorld()->lattice->nbModules;
    exportFile->write((char*)&nbModules, sizeof(size_t));
    if (debug) {
        *debugFile << "-- BEGIN KEY FRAME #" << keyFramesIndex.size()
                   << " (t = " << date << ") --"  << endl;
        *debugFile << nbModules << endl;
    }

    for (const pair<bID, BuildingBlock*>& pair:BaseSimulator::getWorld()->buildingBlocksMap) {
        pair.second->serialize(*exportFile);

        if (debug) pair.second->serialize_cleartext(*debugFile);
    }

    if (debug) {
        *debugFile << "-- END KEY FRAME #" << keyFramesIndex.size() << endl;
    }
}

void ReplayExporter::writeColorUpdate(Time date, bID bid, const Color& color) {
    exportFile->write((char*)&date, sizeof(Time));
    exportFile->write((char*)&EVENT_COLOR_UPDATE, sizeof(u1));
    exportFile->write((char*)&bid, sizeof(bID));
    u1 u1color[3];
    for (std::size_t i;i<3; i++) {
        u1color[i] = clamp(static_cast<int>(color.rgba[i] * 255.0), 0, 255);
    }
    exportFile->write((char*)&u1color,3*sizeof(u1));

    if (debug) {
        *debugFile << "Color:" << date << " " << (int)EVENT_COLOR_UPDATE << " " << bid << " " << (int)u1color[0] << " " << (int)u1color[1] << " " << (int)u1color[2] << endl;
    }
}

void ReplayExporter::writeDisplayUpdate(Time date, bID bid, int value) {
    exportFile->write((char*)&date, sizeof(Time));
    exportFile->write((char*)&EVENT_DISPLAY_UPDATE, sizeof(u1));
    exportFile->write((char*)&bid, sizeof(bID));

    // @TODO: bid, value
    if (debug) {
        *debugFile << "Dpy:" << date << " " << EVENT_DISPLAY_UPDATE << " " << bid << " TODO" << endl;
    }
}

void ReplayExporter::writePositionUpdate(Time date, bID bid, const Cell3DPosition& pos, uint8_t orientation) {
    exportFile->write((char*)&date, sizeof(Time));
    exportFile->write((char*)&EVENT_POSITION_UPDATE, sizeof(u1));
    exportFile->write((char*)&bid, sizeof(bID));
    exportFile->write((char*)&pos.pt,3*sizeof(u2));
    exportFile->write((char*)&orientation,sizeof(u1));
    if (debug) {
        *debugFile << "Pos:" << date << " " << (int)EVENT_POSITION_UPDATE << " " << bid << " " << pos[0] << " " << pos[1] << " " << pos[2] << " " << (int)orientation << endl;
    }

}

void ReplayExporter::writeAddModule(Time date, bID bid) {
    exportFile->write((char*)&date, sizeof(Time));
    exportFile->write((char*)&EVENT_ADD_MODULE, sizeof(u1));
    exportFile->write((char*)&bid, sizeof(u2));

    if (debug) {
        *debugFile << "Add:" << date << " " << (int)EVENT_ADD_MODULE << " " << bid << endl;
    }
}

void ReplayExporter::writeRemoveModule(Time date, bID bid) {
    exportFile->write((char*)&date, sizeof(Time));
    exportFile->write((char*)&EVENT_REMOVE_MODULE, sizeof(u1));
    exportFile->write((char*)&bid, sizeof(bID));

    if (debug) {
        *debugFile << "Rmv:" << date << " " << (int)EVENT_REMOVE_MODULE << " " << bid << endl;
    }
}

void ReplayExporter::writeMotion(Time date, bID bid, Time duration_us,
                                 const Cell3DPosition& destination) {
    exportFile->write((char*)&date, sizeof(Time));
    exportFile->write((char*)&EVENT_MOTION, sizeof(u1));
    exportFile->write((char*)&bid, sizeof(bID));

    // @TODO: bid, duration, destination

    if (debug) {
        *debugFile << "Motion:" << date << " " << (int)EVENT_MOTION << " " << bid << endl;
    }
}

void ReplayExporter::writeConsoleTrace(Time date, bID bid, const string& trace) {
    /*exportFile->write((char*)&date, sizeof(Time));
    exportFile->write((char*)&EVENT_CONSOLE_TRACE, sizeof(u1));
    exportFile->write((char*)&bid, sizeof(bID));
    exportFile->write((char*)&trace.cstr(), trace.length()*sizeof(u1));


    if (debug) {
        *debugFile << "INFO:" << date << " " <<(int) EVENT_MOTION << " " << bid << " " << trace << endl;
    }*/
}
