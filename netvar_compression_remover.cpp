#include "eiface.h"
#include "server_class.h"
#include "dt_common.h"
#include "iostream"

class NetvarDecompressor : public IServerPluginCallbacks
{
public:
	NetvarDecompressor() { };
	~NetvarDecompressor() { };
	virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory);
	virtual void Unload(void) { };
	virtual void Pause(void) { };
	virtual void UnPause(void) { };
	virtual const char *GetPluginDescription(void);
	virtual void LevelInit(char const *pMapName) { };
	virtual void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) { };
	virtual void GameFrame(bool simulating) { };
	virtual void LevelShutdown(void) { };
	virtual void ClientActive(edict_t *pEntity) { };
	virtual void ClientFullyConnect(edict_t *pEntity) { };
	virtual void ClientDisconnect(edict_t *pEntity) { };
	virtual void ClientPutInServer(edict_t *pEntity, char const *playername) { };
	virtual void SetCommandClient(int index) { };
	virtual void ClientSettingsChanged(edict_t *pEdict) { };
	virtual PLUGIN_RESULT ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) { return PLUGIN_CONTINUE; };
	virtual PLUGIN_RESULT ClientCommand(edict_t *pEntity, const CCommand &args) { return PLUGIN_CONTINUE; };
	virtual PLUGIN_RESULT NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) { return PLUGIN_CONTINUE; };
	virtual void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) { };
	virtual void OnEdictAllocated(edict_t *edict) { };
	virtual void OnEdictFreed(const edict_t *edict) { };
	virtual bool BNetworkCryptKeyCheckRequired(uint32 unFromIP, uint16 usFromPort, uint32 unAccountIdProvidedByClient, bool bClientWantsToUseCryptKey) { return false; };
	virtual bool BNetworkCryptKeyValidate(uint32 unFromIP, uint16 usFromPort, uint32 unAccountIdProvidedByClient, int nEncryptionKeyIndexFromClient, int numEncryptedBytesFromClient, byte *pbEncryptedBufferFromClient, byte *pbPlainTextKeyForNetchan) { return false; };
};

ICvar *g_pCVar = NULL;

NetvarDecompressor g_EmptyServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(NetvarDecompressor, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_EmptyServerPlugin);

int g_PLID = 0;

IServerGameDLL *gamedll = NULL;

const char *NetvarDecompressor::GetPluginDescription(void)
{
	return "Netvar compression remover by brooks + emily";
}

void PlayerFlagBitsPatch(const SendProp *pProp, const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID)
{
        int data = *(int *)pVarData;
        pOut->m_Int = (data);
}

void CorrectProps(SendTable *table) 
{
	int numProps = table->GetNumProps();
	for (int i = 0; i < numProps; i++) 
	{
		SendProp* prop = table->GetProp(i);
		if (prop->GetDataTable() && prop->GetNumElements() > 0) 
		{
			if (std::string(prop->GetName()).substr(0, 1) == std::string("0"))
				continue;
			CorrectProps(prop->GetDataTable());
		}

		auto flags = prop->GetFlags();
		if (flags & SPROP_COORD) // COORD is used for vectors and angles, converts the decimal part down to 5bit and integer down to 11bit iirc
			flags &= ~SPROP_COORD;
		flags |= SPROP_NOSCALE;
		prop->SetFlags(flags);
		switch (prop->GetType()) 
		{
			case DPT_Int:
			case DPT_Float:
				if (prop->m_nBits != 32) // floats and integers are 32bit
					prop->m_nBits = 32;
				break;
		}
		if (V_stricmp(prop->GetName(), "m_fFlags") == 0)
                        prop->SetProxyFn(PlayerFlagBitsPatch);
	}
}

bool NetvarDecompressor::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	gamedll = (IServerGameDLL *)gameServerFactory(INTERFACEVERSION_SERVERGAMEDLL, NULL);
	if (!gamedll)
	{
		Warning("Failed to get a pointer on ServerGameDLL.\n");
		return false;
	}
	g_pCVar = (ICvar *)interfaceFactory(CVAR_INTERFACE_VERSION, NULL);
	if (!g_pCVar)
	{
		Warning("Failed to get a pointer on ICvar.\n");
		return false;
	}
	
	static ConVar* sv_sendtables = g_pCVar->FindVar("sv_sendtables");
        sv_sendtables->SetValue(2);
	
	ServerClass *sc = gamedll->GetAllServerClasses();
	while (sc)
	{
		SendTable *table = sc->m_pTable;
		CorrectProps(table);
		sc = sc->m_pNext;
	}

	return true;
}
