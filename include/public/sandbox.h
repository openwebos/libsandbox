//  @@@LICENSE
//
//      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
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

#ifndef SANDBOX_HEADER_INCLUDED
#define SANDBOX_HEADER_INCLUDED 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	c_false,
	c_true
} c_bool;

typedef short int SandboxAction;

#define SB_NONE 0x0
#define SB_READ 0x1
#define SB_WRITE 0x2
#define SB_DELETE 0x4
#define SB_CREATE 0x8
#define SB_ANY 0xFF

#ifndef SANDBOX_INTERNAL
// external only defs here

extern c_bool SBRunAs(uid_t uid, gid_t gid);

extern c_bool SBRunAsLuna();

extern c_bool SBClearEnv();

extern char *SBCanonicalizePath(const char* path);

extern c_bool SBGetAppRoot(const char *appId, char *buffer, int bufferLength);
	
extern c_bool SBGetAppScratch(const char *appId, char *buffer, int bufferLength);

extern c_bool SBIsPathAllowed(const char *path, const char *appId, SandboxAction action);

extern const char *SBGetSandboxActionString(SandboxAction action);

#endif
#ifdef __cplusplus
}
#endif

#endif
