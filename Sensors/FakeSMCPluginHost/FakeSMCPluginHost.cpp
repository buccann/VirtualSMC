//
//  FakeSMCDevice.cpp
//  FakeSMCDevice
//
//  Copyright © 2018 usrsse2. All rights reserved.
//

#include <VirtualSMCSDK/kern_vsmcapi.hpp>

#include "FakeSMCPluginHost.hpp"
#include "FakeSMCValue.hpp"

OSDefineMetaClassAndStructors(FakeSMCDevice, IOService)

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

IOService *FakeSMCDevice::probe(IOService *provider, SInt32 *score) {
	auto ptr = IOService::probe(provider, score);
	if (!ptr) {
		SYSLOG("fsmc", "failed to probe the parent");
		return nullptr;
	}
	
	return this;
}

bool FakeSMCDevice::start(IOService *provider) {
	if (!IOService::start(provider)) {
		SYSLOG("fsmc", "failed to start the parent");
		return false;
	}
	
	registerService();
	
	return true;
}

bool FakeSMCDevice::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
	if (sensors && vsmc) {
		DBGLOG("fsmc", "got vsmc notification");
		auto &plugin = static_cast<FakeSMCDevice *>(sensors)->vsmcPlugin;
		auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &plugin, nullptr, nullptr);
		if (ret == kIOReturnSuccess) {
			DBGLOG("fsmc", "submitted plugin");
			return true;
		} else if (ret != kIOReturnUnsupported) {
			SYSLOG("fsmc", "plugin submission failure %X", ret);
		} else {
			DBGLOG("fsmc", "plugin submission to non vsmc");
		}
	} else {
		SYSLOG("fsmc", "got null vsmc notification");
	}
	return false;
}

void FakeSMCDevice::stop(IOService *provider) {
	PANIC("fsmc", "called stop!!!");
}

IOReturn FakeSMCDevice::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4 )
{
	// special case for locks
	/*
	if (functionName->isEqualTo(kFakeSMCLock)) {
		IORecursiveLockLock(device_lock);
		return kIOReturnSuccess;
	}
	else if (functionName->isEqualTo(kFakeSMCUnlock)) {
		IORecursiveLockUnlock(device_lock);
		return kIOReturnSuccess;
	}
	
	// normal case: lock going in, unlock going out
	IORecursiveLockLock(device_lock);
	 */
	IOReturn result = kIOReturnError;
/*
	if (functionName->isEqualTo(kFakeSMCSetKeyValue)) {
		const char *name = (const char *)param1;
		unsigned char size = (UInt64)param2;
		const void *data = (const void *)param3;
		
		if (name && data && size > 0) {
			
			FakeSMCKey *key = OSDynamicCast(FakeSMCKey, getKey(name));
			if (key && key->setValueFromBuffer(data, size))
				result = kIOReturnSuccess;
		}
		else {
			result = kIOReturnBadArgument;
		}
	}
	else*/
	if (functionName->isEqualTo(kFakeSMCAddKeyHandler)) {
		const char *name = (const char *)param1;
		const char *type = (const char *)param2;
		unsigned char size = (UInt64)param3;
		IOService *handler = (IOService *)param4;
		
		if (name && type && size > 0) {
			
			DBGLOG("fsmc", "adding key %s with handler, type %s, size %d", name, type, size);
			
			// Should be added in sorted order!
			VirtualSMCAPI::addKey(SMC_MAKE_KEY(name[0], name[1], name[2], name[3]),
								  vsmcPlugin.data,
								  new FakeSMCValue(size, SMC_MAKE_KEY_TYPE(type[0], type[1], type[2], type[3]), handler, name));
			result = kIOReturnSuccess;
		}
		else {
			result = kIOReturnBadArgument;
		}
	}
	else if (functionName->isEqualTo(kFakeSMCAddKeyValue)) {
		const char *name = (const char *)param1;
		const char *type = (const char *)param2;
		unsigned char size = (UInt64)param3;
		const void *value = (const void *)param4;
		
		if (name && type && size > 0) {
			
			DBGLOG("fsmc", "adding key %s with value, type %s, size %d", name, type, size);
			
			// Should be added in sorted order!
			VirtualSMCAPI::addKey(SMC_MAKE_KEY(name[0], name[1], name[2], name[3]),
								  vsmcPlugin.data,
								  VirtualSMCAPI::valueWithData(reinterpret_cast<const SMC_DATA*> (value),
															   size,
															   SMC_MAKE_KEY_TYPE(type[0], type[1], type[2], type[3])));
			//if (addKeyWithValue(name, type, size, value))
			result = kIOReturnSuccess;
		}
		else {
			result = kIOReturnBadArgument;
		}
	}/*
	else if (functionName->isEqualTo(kFakeSMCGetKeyValue)) {
		const char *name = (const char *)param1;
		UInt8 *size = (UInt8*)param2;
		const void **value = (const void **)param3;
		
		if (name) {
			if (FakeSMCKey *key = getKey(name)) {
				*size = key->getSize();
				*value = key->getValue();
				
				result = kIOReturnSuccess;
			}
		}
		else {
			return kIOReturnBadArgument;
		}
	}
	else if (functionName->isEqualTo(kFakeSMCRemoveHandler)) {
		if (IOService *handler = (IOService *)param1) {
			if (OSCollectionIterator *iterator = OSCollectionIterator::withCollection(keys)) {
				while (FakeSMCKey *key = OSDynamicCast(FakeSMCKey, iterator->getNextObject()))
					if (key->getHandler() == handler) {
						key->setHandler(NULL);
						result = kIOReturnSuccess;
						break;
					}
				iterator->release();
			}
		}
		else {
			result = kIOReturnBadArgument;
		}
	}*/
	else if (functionName->isEqualTo(kFakeSMCSubmitPlugin)) {
		vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
	}
	/*else {
		result = kIOReturnUnsupported;
	}
	*/
	result = kIOReturnSuccess;
	//IORecursiveLockUnlock(device_lock);
	
	return result;
}


EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
	// Report success but actually do not start and let I/O Kit unload us.
	// This works better and increases boot speed in some cases.
	PE_parse_boot_argn("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
	ADDPR(debugEnabled) = checkKernelArgument("-vsmcdbg");
	return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) {
	// It is not safe to unload VirtualSMC plugins!
	return KERN_FAILURE;
}
