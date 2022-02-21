#include "eiface.h"
#include "server_class.h"
#include "dt_common.h"

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
	return "Netvar compression remover by brooks";
}

bool NetvarDecompressor::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	gamedll = (IServerGameDLL *)gameServerFactory("ServerGameDLL005", NULL);
	if (!gamedll)
	{
		Warning("Failed to get a pointer on ServerGameDLL.\n");
		return false;
	}

	ServerClass *sc = gamedll->GetAllServerClasses();
	while (sc)
	{
		Msg("\x1b[32mTable Name = %s\n", sc->GetName());
		SendTable *table = sc->m_pTable;
		int numProps = table->GetNumProps();
		for (int i = 0; i < numProps; i++)
		{
			SendProp *prop = table->GetProp(i);
			Msg("\x1b[94mProp Name = %s\n", prop->GetName());
			if (prop->flag & SPROP_COORD) // COORD is used for vectors and angles, converts the decimal part down to 5bit and integer down to 11bit iirc
				prop->flag &= ~SPROP_COORD;
			switch (prop->type) {
			FEILD_INTEGER:
			FEILD_FLOAT:
				if (prop->bits != 32) { // floats and integers are 32bit
					prop->bits = 32;
					Msg("\x1b[94mProp %s fixed\n", prop->GetName());
				}

				break;
			}
		}
		sc = sc->m_pNext;
	}

	return true;
}