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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sandbox.h"

int
main(int argc, char **argv)
{
	char *appId = NULL;
	char *path = NULL;
	char *appName = NULL;
	char buffer[1024];
	SandboxAction action = SB_READ;
	
	if (argc < 2) {
		fprintf(stderr, "usage: %s path [appId] [read|write]\n", argv[0]);
		return 1;
	}
	
	path = argv[1];
	
	if (argc > 2) {
		appId=argv[2];
		appName = appId;
	} else 
		appName = strdup("<none>");
		
	if (argc > 3) {
		if  (strcmp(argv[3],"write") == 0)
		action = SB_WRITE;
		else if (strcmp(argv[3],"delete") == 0)
		action = SB_DELETE;
		else if (strcmp(argv[3],"create") == 0)
		action = SB_CREATE;
	}
		
	if (SBRunAsLuna())
		fprintf(stdout, "Running as luna user (good!)\n/tmp/i_ran_as should be owned by luna (ignore the group, it's a /tmp thing)\n");
	else
		fprintf(stdout, "Could not run as luna (bad!)\n/tmp/i_ran_as should be owned by root (and that's bad)\n");
		
	system("/bin/touch /tmp/i_ran_as");
	
	if (SBGetAppRoot(appName, buffer, 1024))
		fprintf(stdout, "Application root for %s is at %s\n", appId, buffer);
	else
		fprintf(stdout, "Could not determine application root for %s\n", appId);
		
	if (SBGetAppScratch(appName, buffer, 1024))
		fprintf(stdout, "Application data dir for %s is at %s\n", appId, buffer);
	else
		fprintf(stdout, "Could not determine application data dir for %s\n", appId);

	if (SBIsPathAllowed(path, appName, action))  {
		fprintf(stdout, "%s is allowed for %s by %s\n", path, SBGetSandboxActionString(action), appName);
		return 0;
	}
	
	fprintf(stdout, "%s is not allowed for %s by %s\n", path, SBGetSandboxActionString(action), appName);
    return 1;
}
