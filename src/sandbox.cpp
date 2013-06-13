//  @@@LICENSE
//
//      Copyright (c) 2010-2013 LG Electronics, Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
//  LICENSE@@@ */

#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <syslog.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <errno.h>

#define SANDBOX_INTERNAL
#define LUNA_USER "luna"
#define LUNA_GROUP "luna"

#include "sandbox.h"
#include "paths.h"

//#define CP_DEBUG
//#define CP_TRACE

#ifdef CP_DEBUG
#define DBG(fmt,...) syslog(LOG_ERR, fmt, __VA_ARGS__);
#else
#define DBG(fmt,...)
#endif

#ifdef CP_TRACE
#define TRC(fmt,...) syslog(LOG_ERR, fmt, __VA_ARGS__);
#else
#define TRC(fmt,...)
#endif

extern char **environ;

#ifndef WEBOS_TARGET_MACHINE_IMPL_SIMULATOR

static bool
pathPrefixCompare(const char *s, const char *t)
{
	int i = 0;
	
	while (s[i] != 0) {
		if (t[i] == 0) {
			// /tmp and /tmp/ should be treated the same
			if (s[i] == '/' && s[i+1] == 0)
				return true;
			else
				return false;
		}
		if (s[i] != t[i])
			return false;
		i++;
	}
	return true;
}

static bool
isPublicPath(const char *path, SandboxAction action)
{
	DBG("isPublicPath? %s\n", path);
	for (int i = 0; i < publicPathsLen; i++) {
			if (pathPrefixCompare(publicPaths[i].path, path)) {
				if (publicPaths[i].allowedAction == SB_ANY || (publicPaths[i].allowedAction & action)) {
					DBG("isPublicPath! [%s]\n",path);
					return true;
				}
			}
	}
	return false;
}

static bool
isPrivilegedPath(const char *path, SandboxAction action)
{
	DBG("isPrivilegedPath? %s\n", path);
	for (int i = 0; i < privilegedPathsLen; i++) {
			if (pathPrefixCompare(privilegedPaths[i].path, path)) {
				if (privilegedPaths[i].allowedAction == SB_ANY || (privilegedPaths[i].allowedAction & action)) {
					DBG("isPrivilegedPath! [%s]\n",path);
					return true;
				}
			}
	}
	return false;
}

static bool
isAppPrivatePath(const char *path, const char *appId, SandboxAction action)
{
	int i = 0;
	int n = 0;
	bool temp = true;
	
	DBG("isAppPrivatePath? %s\n", path);
	
	if (appId == NULL)
		return false;
	
	if (action != SB_READ)
		return false;
	
	while(appPrivatePathPrefix[i] != 0) {
		if ((path[i] == 0)  || (appPrivatePathPrefix[i] != path[i])) {
			temp = false;
			break;
		}
		i++;
	}
	
	if (! temp) {
		i=0;
		while(appPrivatePathPrefixUsr[i] != 0) {
			if ((path[i] == 0)  || (appPrivatePathPrefixUsr[i] != path[i])) {
				return false;
			}
			i++;
		}
	}
	
	while(appId[n] != 0) {
		if (path[i] == 0)
			return false;
		if (path[i] != appId[n]) {
				return false;
		}
		i++;
		n++;
	}

	return true;
}

static bool
isPrivilegedAppId(const char *appId)
{
	if (appId == NULL || appId[0] == 0)
		return false;
		
	// hard code known exceptions here
	if (strncmp(appId, "com.palm.", 9) == 0)
		return true;
	
	return false;
}

#endif

//////////////// external interfaces ///////////////////////

extern "C" {

c_bool 
SBClearEnv(void) {
	static char *namebuf = NULL;
	static size_t lastlen = 0;

	while (environ != NULL && environ[0] != NULL) {
		size_t len = strcspn(environ[0], "=");
		if (len == 0) {
		/* Handle empty variable name (corrupted environ[]) */
		}
		if (len > lastlen) {
			namebuf = (char*)realloc(namebuf, len+1);
			if (namebuf == NULL) {
				syslog(LOG_ERR, "error unsetting environment variable, allocation failure");
			}
			lastlen = len;
		}
		memcpy(namebuf, environ[0], len);
		namebuf[len] = '\0';
		if (unsetenv(namebuf) == -1) {
			syslog(LOG_ERR, "error unsetting environment variable %s: %d", namebuf, errno);
		}
	}
	return c_true;
}

c_bool
SBRunAs(uid_t uid, gid_t gid) 
{
	
	if (setgid(gid) != 0) {
		syslog(LOG_ERR, "failed to set GID %d", gid);
		return c_false;
	}
	if (setuid(uid) != 0) {
		syslog(LOG_ERR, "failed to set UID %d", uid);
		return c_false;
	}
	
	return c_true;
}

c_bool
SBRunAsLuna() 
{
	struct passwd *pwent;
	
	if ((pwent=getpwnam(LUNA_USER)) == NULL) {
		syslog(LOG_ERR, "failed to lookup %s: %d", LUNA_USER, errno);
		return c_false;
	}
	
	if (setgid(pwent->pw_gid) != 0) {
		syslog(LOG_ERR, "failed to set GID %d", pwent->pw_gid);
		return c_false;
	}
	if (setuid(pwent->pw_uid) != 0) {
		syslog(LOG_ERR, "failed to set UID %d", pwent->pw_uid);
		return c_false;
	}
	
	syslog(LOG_INFO, "running as uid:%d, guid:%d", pwent->pw_uid, pwent->pw_gid); 
	
	return c_true;
}


char *
SBCanonicalizePath(const char* mypath)
{	
	DBG("SBCanonicalizePath? %s\n", mypath);
	char *p = canonicalize_file_name(mypath);	
	DBG("SBCanonicalizePath! %s-> %s\n", mypath, s.c_str());
	return p;
}

c_bool
SBGetAppRoot(const char *appId, char *buffer, int bufferLength)
{
	int n = strlen(appPrivatePathPrefix);
	int m = strlen(appId);
	
	if ((n+m) >= bufferLength)
		return c_false;
	
	strncpy(buffer, appPrivatePathPrefix, n);
	strncpy(buffer+n, appId, m);
	buffer[n+m] = NULL;
	
	return c_true;
}

c_bool
SBGetAppScratch(const char *appId, char *buffer, int bufferLength)
{
	int n = strlen(appScratchPathPrefix);
	int m = strlen(appId);
	
	if ((n+m) >= bufferLength)
		return c_false;
	
	strncpy(buffer, appScratchPathPrefix, n);
	strncpy(buffer+n, appId, m);
	buffer[n+m] = NULL;
	
	return c_true;
}

const char *
SBGetSandboxActionString(SandboxAction action) 
{
	switch (action) {
		case SB_READ: return "read";
		case SB_WRITE: return "write";
		case SB_ANY: return "any";
	}
	return "unknown";
}

#ifdef WEBOS_TARGET_MACHINE_IMPL_SIMULATOR
c_bool 
SBIsPathAllowed(const char *, const char *, SandboxAction)
{
        return c_true;
}
#else
c_bool 
SBIsPathAllowed(const char *path, const char *appId, SandboxAction action)
{
	bool isPrivileged = false;
	char *canonicalPath = NULL;
	c_bool isAllowed = c_false;
	
	if (path == NULL) return c_false;
	
	DBG("checking access for path [%s]\n",path);
	
	isPrivileged = isPrivilegedAppId(appId);
	
	DBG("appId is%s privileged\n", isPrivileged?"":" not");
	
	if ((canonicalPath = SBCanonicalizePath(path)) == NULL) {
		isAllowed = c_false;
		goto done;
	}
	
	if (isPrivileged && isPrivilegedPath(canonicalPath, action)) {
		isAllowed = c_true;
		goto done;
	}
		
	if (isPublicPath(canonicalPath, action)) {
		isAllowed = c_true;
		goto done;
	}
		
	if (isAppPrivatePath(canonicalPath, appId, action)) {
		isAllowed = c_true;
		goto done;
	}
		
done:	
	if (canonicalPath != NULL) free(canonicalPath);
	return isAllowed;
}
#endif

}
