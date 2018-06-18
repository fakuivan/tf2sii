/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"
#include "KeyValues.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

#define CONFIG_FILE_NAME	"tf2sii"
#define CONFIG_FILE_KV		"CTFItemSchema::m_pKeyValues"
#define CONFIG_FILE_GII		"GetItemSchema"

TF2SimpleItemInfo g_TF2SimpleItemInfo;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_TF2SimpleItemInfo);

struct {
	typedef void* (*func)();
	int nKvOffset;
	void* pGetItemSchema;
	bool bReady;
	KeyValues * operator()() {
		if (!bReady) { return NULL; }
		auto pItemSchema = ((func)pGetItemSchema)();
		if (pItemSchema == NULL) { return NULL; }
		return *((KeyValues **)((char*)(pItemSchema) + nKvOffset));
	}
} GetItemSchemaKV;

bool TF2SimpleItemInfo::SDK_OnLoad(char * error, size_t maxlength, bool late)
{
	GetItemSchemaKV.bReady = false;
	IGameConfig *gameConfig = NULL;
	auto configError = new char[maxlength];
	if (!gameconfs->LoadGameConfigFile(CONFIG_FILE_NAME, &gameConfig, configError, maxlength))
	{
		snprintf(error, maxlength, "Failed to read config file '" CONFIG_FILE_NAME "': %s\n", configError);
		return false;
	}
	delete configError;

	if (!gameConfig->GetMemSig(CONFIG_FILE_GII, &(GetItemSchemaKV.pGetItemSchema)))
	{
		snprintf(error, maxlength, "Failed to find section '" CONFIG_FILE_GII "' for signature\n");
		return false;
	}
	if (GetItemSchemaKV.pGetItemSchema == NULL)
	{
		snprintf(error, maxlength, "Failed to find location on memory for signature " CONFIG_FILE_GII "\n");
		return false;
	}
	
	if (!gameConfig->GetOffset(CONFIG_FILE_KV, &(GetItemSchemaKV.nKvOffset)))
	{
		snprintf(error, maxlength, "Failed to find section '" CONFIG_FILE_KV "' for offset\n");
		return false;
	}
	
	GetItemSchemaKV.bReady = true;
	auto pItemSchemaKV = GetItemSchemaKV();
	if (pItemSchemaKV == NULL)
	{
		snprintf(error, maxlength, "Failed to get property 'CTFItemSchema::m_pKeyValues'\n");
		return false;
	}
	g_pSM->LogMessage(myself, "Section name for item schema: '%s'.", pItemSchemaKV->GetName());
	return true;
}
