//
//  AppleProtocols.c
//  Clover
//
//  Created by Slice on 14.10.16.
//

#include <AppleUefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/UgaDraw.h>
#include <Protocol/AppleSmcIo.h>
#include <Protocol/AppleImageCodecProtocol.h>
#include <Protocol/AppleKeyState.h>
#include <Protocol/OSInfo.h>
//#include <Protocol/AppleEvent.h>
#include <Protocol/FirmwareVolume.h>

#include "Common.h"

//#define APPLE_SMC_IO_PROTOCOL_GUID         {0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}}
//#define APPLE_IMAGE_CODEC_PROTOCOL_GUID	{0x0dfce9f6, 0xc4e3, 0x45ee, 0xa0, 0x6a, 0xa8, 0x61, 0x3b, 0x98, 0xa5, 0x07}

/****************************************************/
/**/
/** Installs our AppleSMC overrides. */

/** Original DataHub protocol. */
APPLE_SMC_IO_PROTOCOL gOrgAppleSMC;
APPLE_SMC_IO_PROTOCOL *gAppleSMC;

EFI_STATUS EFIAPI
OvrReadData (
  IN APPLE_SMC_IO_PROTOCOL *This,
  IN SMC_KEY Key,
  IN SMC_DATA_SIZE Size,
  OUT SMC_DATA *Value
  )
{
  EFI_STATUS Status;

  Status = gOrgAppleSMC.SmcReadValue (This, Key, Size, Value);

  PRINT (
    "->AppleSMC.SmcReadValue SMC=%x (%c%c%c%c) len=%d\n",
    Key, (Key >> 24) & 0xFF,
    (Key >> 16) & 0xFF,
    (Key >> 8) & 0xFF,
    Key & 0xFF, Size
    );

  return Status;
}

EFI_STATUS
EFIAPI
OvrAppleSMC (
  VOID
)
{
  EFI_STATUS Status;

  PRINT ("Overriding AppleSMC ...\n");

  // Locate AppleSMC protocol
  Status = gBS->LocateProtocol (
                  &gAppleSmcIoProtocolGuid,
                  NULL,
                  (VOID **)&gAppleSMC
                  );

  if (EFI_ERROR (Status)) {
    PRINT ("Error Overriding AppleSMC: %r\n", Status);
    return Status;
  }

  // Store originals
  CopyMem (&gOrgAppleSMC, gAppleSMC, sizeof(APPLE_SMC_IO_PROTOCOL));

  // Override with our implementation
  gAppleSMC->SmcReadValue = OvrReadData;

  PRINT ("AppleSMC overriden!\n");
  return EFI_SUCCESS;
}

/****************************************************/
/**/
/** Installs our AppleImageCodecProtocol overrides. */

APPLE_IMAGE_CODEC_PROTOCOL gOrgAppleImageCodec;
APPLE_IMAGE_CODEC_PROTOCOL *gAppleImageCodec;

EFI_STATUS
EFIAPI
OvrRecognizeImageData (
  IN APPLE_IMAGE_CODEC_PROTOCOL* This,
                    VOID         *ImageBuffer,
                    UINTN         ImageSize
                    )
{
  EFI_STATUS				Status;

  Status = gOrgAppleImageCodec.RecognizeImageData(This, ImageBuffer, ImageSize);
  PRINT("->RecognizeImageData(%p, 0x%x), sign=%4x, status=%r\n", ImageBuffer, ImageSize,
        ImageBuffer?(*(UINT32*)ImageBuffer):0, Status);
  return Status;
}

EFI_STATUS
EFIAPI
OvrGetImageDims (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
              VOID          *ImageBuffer,
              UINTN         ImageSize,
              UINT32         *ImageWidth,
              UINT32         *ImageHeight
              )
{
  EFI_STATUS				Status;

  Status = gOrgAppleImageCodec.GetImageDims(This, ImageBuffer, ImageSize, ImageWidth, ImageHeight);
  PRINT("->GetImageDims(%p, 0x%x, %p, %p), status=%r\n", ImageBuffer, ImageSize, ImageWidth, ImageHeight, Status);
  if (!EFI_ERROR(Status)) {
    PRINT("--> ImageWidth=%d, ImageHeight=%d\n", ImageWidth?*ImageWidth:0, ImageHeight?*ImageHeight:0);
  }
  return Status;
}

EFI_STATUS
EFIAPI
OvrDecodeImageData (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
                 VOID          *ImageBuffer,
                 UINTN         ImageSize,
                 EFI_UGA_PIXEL **RawImageData,
                 UINTN         *RawImageDataSize
                 )
{
  EFI_STATUS				Status;

  Status = gOrgAppleImageCodec.DecodeImageData(This, ImageBuffer, ImageSize, RawImageData, RawImageDataSize);
  PRINT("->DecodeImageData(%p, 0x%x, %p, %p), status=%r\n", ImageBuffer, ImageSize, RawImageData, RawImageDataSize, Status);
  if (!EFI_ERROR(Status)) {
    PRINT("--> RawImageDataSize=%d\n", RawImageDataSize?*RawImageDataSize:0);
  }
  return Status;
}

EFI_STATUS
EFIAPI
OvrUnknown (IN APPLE_IMAGE_CODEC_PROTOCOL* This,
                       VOID         *ImageBuffer,
                       UINTN         ImageSize
                       )
{
  EFI_STATUS				Status;

  Status = gOrgAppleImageCodec.Unknown(This, ImageBuffer, ImageSize);
  PRINT("->UnknownCall(%p, 0x%x), status=%r\n", ImageBuffer, ImageSize, Status);
  return Status;
}


EFI_STATUS EFIAPI
OvrAppleImageCodec(VOID)
{
	EFI_STATUS				Status;

	PRINT("Overriding AppleImageCodec ...\n");

	// Locate AppleSMC protocol
	Status = gBS->LocateProtocol(&gAppleImageCodecProtocolGuid, NULL, (VOID **) &gAppleImageCodec);
	if (EFI_ERROR(Status)) {
		PRINT("Error Overriding AppleImageCodec: %r\n", Status);
		return Status;
	}

	// Store originals
	CopyMem(&gOrgAppleImageCodec, gAppleImageCodec, sizeof(APPLE_IMAGE_CODEC_PROTOCOL));

	// Override with our implementation
	gAppleImageCodec->RecognizeImageData = OvrRecognizeImageData;
	gAppleImageCodec->GetImageDims = OvrGetImageDims;
	gAppleImageCodec->DecodeImageData = OvrDecodeImageData;
  gAppleImageCodec->Unknown = OvrUnknown;

	PRINT("AppleImageCodec overriden!\n");
	return EFI_SUCCESS;
}

/****************************************************/
/**/
/** Installs our AppleKeyState overrides. */

APPLE_KEY_STATE_PROTOCOL gOrgAppleKeyState;
APPLE_KEY_STATE_PROTOCOL *gAppleKeyState;

EFI_STATUS
EFIAPI
OvrReadKeyState (IN APPLE_KEY_STATE_PROTOCOL *This,
                 OUT UINT16 *ModifyFlags,
                 OUT UINTN  *PressedKeyStatesCount,
                 OUT CHAR16 *PressedKeyStates)
{
  EFI_STATUS				Status;

  Status = gOrgAppleKeyState.ReadKeyState(This, ModifyFlags, PressedKeyStatesCount, PressedKeyStates);
  PRINT("->ReadKeyState(), count=%d, flags=0x%x states=%s, status=%r\n",
        PressedKeyStatesCount?*PressedKeyStatesCount:0,
        ModifyFlags?*ModifyFlags:0,
        PressedKeyStates, Status);
  return Status;
}



EFI_STATUS EFIAPI
OvrAppleKeyState(VOID)
{
  EFI_STATUS				Status;

  PRINT("Overriding AppleKeyState ...\n");

  // Locate AppleKeyState protocol
  Status = gBS->LocateProtocol(&gAppleKeyStateProtocolGuid, NULL, (VOID **) &gAppleKeyState);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding AppleKeyState: %r\n", Status);
    return Status;
  }

  // Store originals
  CopyMem(&gOrgAppleKeyState, gAppleKeyState, sizeof(APPLE_KEY_STATE_PROTOCOL));

  // Override with our implementation
  gAppleKeyState->ReadKeyState = OvrReadKeyState;

  PRINT("AppleKeyState overriden!\n");
  return EFI_SUCCESS;

}

//**************************************************
EFI_OS_INFO_PROTOCOL gOrgOSInfo;
EFI_OS_INFO_PROTOCOL *gOSInfo;

// OSInfoOSNameImpl
VOID
EFIAPI
OvrOSName (IN CHAR8 *OSName)
{
//  EFI_STATUS				Status;
  gOrgOSInfo.OSName(OSName);
  PRINT("->OSInfo.OSName=%a\n", OSName);
}

// OSInfoOSVendorImpl
VOID
EFIAPI
OvrOSVendor (IN CHAR8 *OSVendor)
{
//  EFI_STATUS				Status;
  gOrgOSInfo.OSVendor(OSVendor);
  PRINT("->OSInfo.OSVendor=%a\n", OSVendor);
}


EFI_STATUS EFIAPI
OvrOSInfo(VOID)
{
  EFI_STATUS				Status;

  PRINT("Overriding OSInfo ...\n");

  // Locate AppleKeyState protocol
  Status = gBS->LocateProtocol(&gEfiOSInfoProtocolGuid, NULL, (VOID **) &gOSInfo);
  if (EFI_ERROR(Status)) {
    PRINT("Error Overriding OSInfo: %r\n", Status);
    return Status;
  }

  // Store originals
  CopyMem(&gOrgOSInfo, gOSInfo, sizeof(EFI_OS_INFO_PROTOCOL));

  // Override with our implementation
  gOSInfo->OSVendor = OvrOSVendor;
  gOSInfo->OSName = OvrOSName;

  PRINT("EfiOSInfo overriden!\n");
  return EFI_SUCCESS;

}
