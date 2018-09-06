//
//  FakeSMCValue.cpp
//  FakeSMCPluginHost
//
//  Created by гык-sse2 on 03/09/2018.
//  Copyright © 2018 vit9696. All rights reserved.
//

#include "FakeSMCValue.hpp"
#include "FakeSMCDefinitions.h"

SMC_RESULT FakeSMCValue::readAccess() {
	if (handler) {
		IOReturn result = handler->callPlatformFunction(kFakeSMCGetValueCallback, true, (void *)name, (void *)data, reinterpret_cast<void*>(size), 0);
		
		if (kIOReturnSuccess != result)
			DBGLOG("fsmc", "Value update request callback error for key %s, return 0x%x", name, result);
	}
	
	return 0;
}
