// SSLProtocol.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SSLProtocol.h"
#include "SSLContext.h"

struct CRYPTO_dynlock_value
{
	CRITICAL_SECTION myLock;
};

CRITICAL_SECTION* pLocks = NULL;
unsigned int nLocks = 0;
char strServerCertificatePrivPassword[MAX_PATH];


void MyLockingCallback(int mode, int type, const char *file, int line) 
{ 
	if (mode & CRYPTO_LOCK) 
		EnterCriticalSection(&pLocks[type]);
	else 
		LeaveCriticalSection(&pLocks[type]);
}

void MyDynamicLockingCallback( int inMode, CRYPTO_dynlock_value * pDynLock, const char * ipsFile,	int inLine )
{
	if ( inMode & CRYPTO_LOCK ) {
		EnterCriticalSection(&pDynLock->myLock );
	} else {
		LeaveCriticalSection(&pDynLock->myLock );
	}
}

void MyDynamicLockingCleanupCallback ( struct CRYPTO_dynlock_value * pDynLock, const char * ipsFile, int inLine )
{
	DeleteCriticalSection(&pDynLock->myLock );
	delete pDynLock;
}

unsigned long id_function(void)
{
	return (unsigned long) GetCurrentThreadId();
}

int password_callback(char *buf, int size, int rwflag, void *userdata) 
{ 
	strcpy_s(buf, size, strServerCertificatePrivPassword); 
	return strlen(strServerCertificatePrivPassword);
}

CRYPTO_dynlock_value * MyDynamicLockingCreateCallback ( const char * ipsFile, int inLine )
{	 
	CRYPTO_dynlock_value * poDynlock = new CRYPTO_dynlock_value;

	InitializeCriticalSection(&poDynlock->myLock );
	return poDynlock;
}

//////////////////////////////////////////////////////////////////////////

SSLProtocol::SSLProtocol( void )
{
	isInitialized = false;
}

SSLProtocol::~SSLProtocol()
{
	if (isInitialized)
	{
		ERR_free_strings();

		CRYPTO_set_dynlock_create_callback ( NULL );
		CRYPTO_set_dynlock_lock_callback ( NULL );
		CRYPTO_set_dynlock_destroy_callback ( NULL );

		if (pLocks != NULL)
		{
			for (int i=0;i<nLocks;i++)
				DeleteCriticalSection(&pLocks[i]);
			delete [] pLocks;
		}
	}
}

void SSLProtocol::initializeLocks()
{
	nLocks = CRYPTO_num_locks();
	pLocks = new CRITICAL_SECTION[nLocks];
	for (int i=0;i<nLocks;i++)
		InitializeCriticalSection(&pLocks[i]);

	CRYPTO_set_locking_callback(MyLockingCallback);
	CRYPTO_set_id_callback(id_function);

	CRYPTO_set_dynlock_create_callback ( MyDynamicLockingCreateCallback );
	CRYPTO_set_dynlock_lock_callback ( MyDynamicLockingCallback );
	CRYPTO_set_dynlock_destroy_callback ( MyDynamicLockingCleanupCallback );
}


bool SSLProtocol::initializeAsServer( const char* serverCertificatePubKey, const char* serverCertificatePrivKey, const char* serverCertificatePrivPassword /*= NULL*/ )
{
	isServer = true;

	if(!initializeCommon())
		return false;

	int ret = SSL_CTX_use_certificate_file(ctx, serverCertificatePubKey, SSL_FILETYPE_PEM);
	if(ret <= 0)
		return false;

	if (serverCertificatePrivPassword != NULL)
	{
		strcpy_s(strServerCertificatePrivPassword, MAX_PATH, serverCertificatePrivPassword);
		SSL_CTX_set_default_passwd_cb(ctx, &password_callback);
	}

	ret = SSL_CTX_use_PrivateKey_file(ctx, serverCertificatePrivKey, SSL_FILETYPE_PEM);
	if (ret <= 0)
		return false;

	/* Check if the server certificate and private-key matches */
	ret = SSL_CTX_check_private_key(ctx);
	if (!ret)
		return false;

	return true;
}

bool SSLProtocol::initializeAsClient( const char* caCertificatePubKey )
{
	isServer = false;

	if(!initializeCommon())
		return false;

	int ret	= SSL_CTX_load_verify_locations(ctx, caCertificatePubKey, NULL);
	if (!ret) {
		return false;
	}

	/* Set flag in context to require peer (server) certificate */
	/* verification */

	SSL_CTX_set_verify(ctx, /*SSL_VERIFY_NONE*/SSL_VERIFY_PEER, NULL);

	SSL_CTX_set_verify_depth(ctx, 1);

	return true;
}

bool SSLProtocol::initializeCommon()
{
	SSL_library_init();
	SSL_load_error_strings();

	// Create the SSL context.
	const SSL_METHOD *meth = SSLv3_method();
	ctx = SSL_CTX_new(meth);
	if (!ctx) 
		return false;

	initializeLocks();

	isInitialized = true;

	return true;
}


ProtocolContext* SSLProtocol::createNewProtocolContext()
{
	return new SSLContext(getMsgBufferPool());
}

void SSLProtocol::startSession( ProtocolContext& context, Buffer& outgoingBytes )
{
	SSLContext& sslContext = (SSLContext&) context;
	sslContext.initializeSSLSession(ctx, isServer, outgoingBytes);
}

bool SSLProtocol::readData( ProtocolContext& context, Buffer& incomingBytes )
{
	SSLContext& sslContext = (SSLContext&) context;
	//
	return sslContext.readData(incomingBytes);
}

DecodeResult::Type SSLProtocol::tryDecode( ProtocolContext& context, Buffer& outputBuffer )
{
	SSLContext& sslContext = (SSLContext&) context;

	return sslContext.tryDecode(outputBuffer);
}

EncodeResult::Type SSLProtocol::encodeContent( ProtocolContext& context, Buffer& inputBuffer, Buffer& outputBuffer )
{
	SSLContext& sslContext = (SSLContext&) context;

	return sslContext.encodeContent(inputBuffer, outputBuffer);
}

unsigned int SSLProtocol::getRequiredOutputSize( unsigned int maxInputSize )
{
	return maxInputSize /** 2*/;
}
