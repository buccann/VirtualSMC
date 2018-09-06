//
//  FakeSMCValue.hpp
//  FakeSMCPluginHost
//
//  Created by гык-sse2 on 03/09/2018.
//  Copyright © 2018 vit9696. All rights reserved.
//

#ifndef FakeSMCValue_hpp
#define FakeSMCValue_hpp

#include <VirtualSMCSDK/kern_vsmcapi.hpp>

class FakeSMCValue : public VirtualSMCValue {
	IOService *handler;
	char name[5];
	
public:
	FakeSMCValue(SMC_DATA_SIZE size, SMC_KEY_TYPE type, IOService *handler, const char* name) : handler(handler) {
		init(nullptr, size, type, SMC_KEY_ATTRIBUTE_READ);
		lilu_os_strncpy(this->name, name, 5);
	}
	
	virtual SMC_RESULT readAccess() override;
};

#endif /* FakeSMCValue_hpp */
