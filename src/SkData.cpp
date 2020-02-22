/******************************************************************************
* $Id: SkData.cpp, v1.0 2019/11/30 VaderDarth Exp $
*
* Project:  OpenCPN
* Purpose:  dahbooard_tactics_pi plug-in
* Author:   Jean-Eudes Onfray
*
***************************************************************************
*   Copyright (C) 2010 by David S. Register   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
***************************************************************************
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/tokenzr.h>

#include "SkData.h"
#include <string>
#include <algorithm>

//************************************************************************************************************************
// Signal K originated data handling / containers / C++/HTML/CSS/JS
//************************************************************************************************************************

SkData::SkData()
{
    m_pathlist         = new SkDataPathList();
    m_nmea0183pathlist = new SkDataPathList();
    m_nmea2000pathlist = new SkDataPathList();
    m_subscriptionlist = new SkDataPathList();
    pushDefaultSubscriptions();
    return;
}

SkData::SkData(const SkData& sourceSkData) {
    m_pathlist         = new SkDataPathList(*sourceSkData.m_pathlist);
    m_nmea0183pathlist = new SkDataPathList(*sourceSkData.m_nmea0183pathlist);
    m_nmea2000pathlist = new SkDataPathList(*sourceSkData.m_nmea2000pathlist);
    m_subscriptionlist = new SkDataPathList(*sourceSkData.m_subscriptionlist);
    m_subscribedToAllPaths = false;
    return;
}

SkData::~SkData()
{
    delete m_pathlist;
    delete m_nmea0183pathlist;
    delete m_nmea2000pathlist;
    delete m_subscriptionlist;
    return;
}

void SkData::pushDefaultSubscriptions()
{
    // Signal K paths to be subscribed for by default and without learning/selection
    m_subscriptionlist->push_back( std::string( "environment.depth.belowTransducer" ) );
    m_subscriptionlist->push_back( std::string( "environment.depth.belowKeel" ) );
    m_subscriptionlist->push_back( std::string( "environment.outside.pressure" ) );
    m_subscriptionlist->push_back( std::string( "environment.water.temperature" ) );
    m_subscriptionlist->push_back( std::string( "environment.wind.angleApparent" ) );
    m_subscriptionlist->push_back( std::string( "environment.wind.angleTrueWater" ) );
    m_subscriptionlist->push_back( std::string( "environment.wind.speedApparent" ) );
    m_subscriptionlist->push_back( std::string( "environment.wind.speedTrue" ) );
    m_subscriptionlist->push_back( std::string( "navigation.courseOverGroundTrue" ) );
    m_subscriptionlist->push_back( std::string( "navigation.courseRhumbline.nextPoint.bearingTrue" ) );
    m_subscriptionlist->push_back( std::string( "navigation.courseRhumbline.nextPoint.velocityMadeGood" ) );
    m_subscriptionlist->push_back( std::string( "navigation.courseRhumbline.nextPoint.distance" ) );
    m_subscriptionlist->push_back( std::string( "navigation.datetime" ) );
    m_subscriptionlist->push_back( std::string( "navigation.gnss.methodQuality" ) );
    m_subscriptionlist->push_back( std::string( "navigation.gnss.satellites" ) );
    m_subscriptionlist->push_back( std::string( "navigation.headingMagnetic" ) );
    m_subscriptionlist->push_back( std::string( "navigation.headingTrue" ) );
    m_subscriptionlist->push_back( std::string( "navigation.magneticVariation" ) );
    m_subscriptionlist->push_back( std::string( "navigation.position" ) );
    m_subscriptionlist->push_back( std::string( "navigation.speedOverGround" ) );
    m_subscriptionlist->push_back( std::string( "navigation.speedThroughWater" ) );
    m_subscriptionlist->push_back( std::string( "navigation.trip.log" ) );
    m_subscriptionlist->push_back( std::string( "navigation.log" ) );
    m_subscriptionlist->push_back( std::string( "steering.rudderAngle" ) );
    m_subscribedToAllPaths = false;
}

void SkData::UpdatePathList( SkDataPathList *pathlist, wxString *path, wxString *key )
{
    std::string stdPathFull = std::string( path->mb_str() );
    if ( key != NULL ) {
        std::string stdKey  = std::string( key->mb_str() );
        stdPathFull = stdPathFull + "." + stdKey;
    }
    SkDataPathList::iterator it = std::find(pathlist->begin(), pathlist->end(), stdPathFull);
    if ( it != pathlist->end() )
        return;
    pathlist->push_back( stdPathFull );
    return;
}

void SkData::UpdateNMEA2000PathList( wxString *path, wxString *key )
{
    if ( path == NULL )
        return;
    UpdatePathList( m_nmea2000pathlist, path, key );
    UpdatePathList( m_pathlist, path, key );
}

void SkData::UpdateNMEA0183PathList( wxString *path, wxString *key )
{
    if ( path == NULL )
        return;
    UpdatePathList( m_nmea0183pathlist, path, key );
    UpdatePathList( m_pathlist, path, key );
}

void SkData::UpdateSubscriptionList( wxString *path, wxString *key )
{
    if ( path == NULL )
        return;
    UpdatePathList( m_subscriptionlist, path, key );
}

wxString SkData::getAllJsOrderedList(
    SkDataPathList *pathlist,
    wxJSONValue &pRetJSON)
{
    SkDataPathList sortedList = *pathlist;
    sortedList.sort();
    SkDataPathList topics;;
    SkDataPathList::iterator it;
    // Search main topics for the ordred sub-lists
    std::string topic = "";
    for (it = sortedList.begin(); it != sortedList.end(); ++it) {
        std::string fullpathkey = *it;
        wxString keyval = wxString( fullpathkey );
        wxStringTokenizer tokenizer(keyval, ".");
        wxString sTopic = tokenizer.GetNextToken();
        std::string gotTopic = std::string( sTopic.mb_str() );
        if ( gotTopic != topic ) {
            topics.push_back( gotTopic );
            topic = gotTopic;
        }
    }
    /* Order the sub-topics under main topics first by the number of sub-topics
       they have and only then alphaetically, to allow tighter menus.
       There shall be from 2 to 6 sub-elemenents, cf. https://git.io/Jep2E */
    SkDataPathList::iterator topicit;
    int maxPathSubElements = 7; // reminder: we have also the key of a value
    wxString retval = "";
    int itemIdx = 0;
    bool notFirstObj = false;
    wxJSONType jsonType = pRetJSON.GetType();
    for (topicit = topics.begin(); topicit != topics.end(); ++topicit) {
        std::string sTopicToCollect = *topicit;
        wxString topicToCollect = wxString( sTopicToCollect );
        for (int i = 1; i < maxPathSubElements; i++ ) {
            for (it = sortedList.begin(); it != sortedList.end(); ++it) {
                std::string sFullPath = *it;
                wxString fullPath = wxString( sFullPath );
                wxStringTokenizer tokenizer(fullPath, ".");
                if ( tokenizer.CountTokens() == (i + 1) ) {
                    wxString nowTopic = tokenizer.GetNextToken();
                    if ( nowTopic == topicToCollect ) {
                        if ( jsonType != wxJSONTYPE_NULL )
                            pRetJSON["subscribe"][itemIdx]["path"] = fullPath;
                        if ( notFirstObj )
                            retval += "," + fullPath;
                        else {
                            retval += fullPath;
                            notFirstObj = true;
                        }
                    itemIdx++;
                    }
                }
            }
        }
    }
    return retval;
}

wxString SkData::getAllNMEA2000JsOrderedList()
{
    wxJSONValue noJSON( wxJSONTYPE_NULL );
    return getAllJsOrderedList( m_nmea2000pathlist, noJSON );
}

wxString SkData::getAllNMEA0183JsOrderedList()
{
    wxJSONValue noJSON( wxJSONTYPE_NULL );
    return getAllJsOrderedList( m_nmea0183pathlist, noJSON );
}

wxString SkData::getAllSubscriptionsJSON(wxJSONValue &pRetJSON)
{
    return getAllJsOrderedList( m_subscriptionlist, pRetJSON );
}