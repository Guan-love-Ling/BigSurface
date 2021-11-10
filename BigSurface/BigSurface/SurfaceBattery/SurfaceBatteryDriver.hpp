//
//  SurfaceBatteryDriver.hpp
//  SurfaceBattery
//
//  Copyright © 2018 usrsse2. All rights reserved.
//
//  Portions copyright © 2009 Superhai.
//

#ifndef SurfaceBatteryDriver_hpp
#define SurfaceBatteryDriver_hpp

#include <IOKit/IOService.h>
#include <IOKit/IOReportTypes.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/pwr_mgt/IOPMPowerSource.h>
#include <IOKit/pwr_mgt/RootDomain.h>
#include <IOKit/IOTimerEventSource.h>

#include "BatteryManager.hpp"
#include "../SurfaceSerialHub/SurfaceSerialHubDriver.hpp"
#include "../SurfaceSerialHub/SerialProtocol.h"

#define PRODUCT_NAME SurfaceBatteryDriver

class EXPORT SurfaceBatteryDriver : public SurfaceSerialHubClient {
	OSDeclareDefaultStructors(SurfaceBatteryDriver)

	/**
	 *  Key name index mapping
	 */
	static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEF") - 1;
	static constexpr const char *KeyIndexes = "0123456789ABCDEF";

	/**
	 *  Key name declarations
	 */
	static constexpr SMC_KEY KeyAC_N = SMC_MAKE_IDENTIFIER('A','C','-','N');
	static constexpr SMC_KEY KeyACEN = SMC_MAKE_IDENTIFIER('A','C','E','N');
	static constexpr SMC_KEY KeyACFP = SMC_MAKE_IDENTIFIER('A','C','F','P');
	static constexpr SMC_KEY KeyACID = SMC_MAKE_IDENTIFIER('A','C','I','D');
	static constexpr SMC_KEY KeyACIN = SMC_MAKE_IDENTIFIER('A','C','I','N');
	static constexpr SMC_KEY KeyBATP = SMC_MAKE_IDENTIFIER('B','A','T','P');
	static constexpr SMC_KEY KeyBBAD = SMC_MAKE_IDENTIFIER('B','B','A','D');
	static constexpr SMC_KEY KeyBBIN = SMC_MAKE_IDENTIFIER('B','B','I','N');
	static constexpr SMC_KEY KeyBFCL = SMC_MAKE_IDENTIFIER('B','F','C','L');
	static constexpr SMC_KEY KeyBNum = SMC_MAKE_IDENTIFIER('B','N','u','m');
	static constexpr SMC_KEY KeyBRSC = SMC_MAKE_IDENTIFIER('B','R','S','C');
	static constexpr SMC_KEY KeyBSIn = SMC_MAKE_IDENTIFIER('B','S','I','n');
	static constexpr SMC_KEY KeyCHBI = SMC_MAKE_IDENTIFIER('C','H','B','I');
	static constexpr SMC_KEY KeyCHBV = SMC_MAKE_IDENTIFIER('C','H','B','V');
	static constexpr SMC_KEY KeyCHLC = SMC_MAKE_IDENTIFIER('C','H','L','C');

	static constexpr SMC_KEY KeyB0AC(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'A','C'); }
	static constexpr SMC_KEY KeyB0AV(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'A','V'); }
	static constexpr SMC_KEY KeyB0BI(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'B','I'); }
	static constexpr SMC_KEY KeyB0CT(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'C','T'); }
	static constexpr SMC_KEY KeyB0FC(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'F','C'); }
	static constexpr SMC_KEY KeyB0PS(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'P','S'); }
	static constexpr SMC_KEY KeyB0RM(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'R','M'); }
	static constexpr SMC_KEY KeyB0St(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'S','t'); }
	static constexpr SMC_KEY KeyB0TF(size_t i) { return SMC_MAKE_IDENTIFIER('B',KeyIndexes[i],'T','F'); }
	static constexpr SMC_KEY KeyD0IR(size_t i) { return SMC_MAKE_IDENTIFIER('D',KeyIndexes[i],'I','R'); }
	static constexpr SMC_KEY KeyD0VM(size_t i) { return SMC_MAKE_IDENTIFIER('D',KeyIndexes[i],'V','M'); }
	static constexpr SMC_KEY KeyD0VR(size_t i) { return SMC_MAKE_IDENTIFIER('D',KeyIndexes[i],'V','R'); }
	static constexpr SMC_KEY KeyD0VX(size_t i) { return SMC_MAKE_IDENTIFIER('D',KeyIndexes[i],'V','X'); }
	static constexpr SMC_KEY KeyTB0T(size_t i) { return SMC_MAKE_IDENTIFIER('T','B',KeyIndexes[i],'T'); }

	// Keys used in BigSur 10.16 and later
	static constexpr SMC_KEY KeyBC1V(size_t i) { return SMC_MAKE_IDENTIFIER('B','C',KeyIndexes[i],'V'); }
	
	/**
	 *  VirtualSMC service registration notifier
	 */
	IONotifier *vsmcNotifier {nullptr};

	VirtualSMCAPI::Plugin vsmcPlugin {
		xStringify(PRODUCT_NAME),
		parseModuleVersion(xStringify(MODULE_VERSION)),
		VirtualSMCAPI::Version,
	};

public:
	IOService *probe(IOService *provider, SInt32 *score) override;
	
	bool start(IOService *provider) override;
	
	void stop(IOService *provider) override;
    
    IOReturn setPowerState(unsigned long whichState, IOService * device) override;
    
    IOReturn setProperties(OSObject* props) override;
	
	static bool vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier);
    
    void eventReceived(UInt8 tc, UInt8 iid, UInt8 cid, UInt8 *data_buffer, UInt16 length) override;
    
private:
    IOWorkLoop*             work_loop {nullptr};
    IOTimerEventSource*     timer {nullptr};
    IOInterruptEventSource* interrupt_source {nullptr};
    SurfaceSerialHubDriver* ssh {nullptr};
    
    bool awake {false};
//    bool battery_full {false};
    SInt8 update_bst_idx {-1};
    SInt8 update_bix_idx {-1};
    SInt8 update_adp_idx {-1};
    
    void updateStatus(OSObject* target, void* refCon, IOService* nubDevice, int source);
};

#endif /* SurfaceBatteryDriver_hpp */
