#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "cvi_exif.h"
#include "cvi_log.h"

unsigned char ExifHeader[6] =
{
    0x45,0x78,0x69,0x66,0x00,0x00
};

unsigned char TIFFHeader[8] =
{
    0x49,0x49,0x2A,0x00,0x08,0x00,0x00,0x00
};

typedef union BytesOfShort
{
    unsigned char b[2];
    unsigned short s;
} BytesOfShort_t;

enum EXTRACT_DATE_JOB_STATE {
    EXTRACT_START = 0,
    READ_EXIF_HEADER = 1,
    READ_CHECK_SAT_HEADER,
    COPY_REMAINING_DATA,
    EXTRACT_END
};

int32_t CVI_EXIF_MakeExifFile(char *ExifOut, uint32_t *totalLen, CVI_EXIF_FILE_INFO_S *exifFileInfo)
{
    unsigned char *ExifInitialCount;
    unsigned char *tempExif = (unsigned char *)ExifOut;
    int32_t ExifSize;
    uint32_t santemp;
    unsigned char *startoftiff;
    unsigned char APP1Marker[2] = {0xff, 0xe1};
    unsigned char ExifLen[4] = {0};
    unsigned char Nentries[2] = {8, 0};
    unsigned char SubIFDNentries[2] = {18, 0};
    unsigned char EndOfEntry[4] = {0};

    // MAKE
    unsigned char maketag[2] = {0xf, 0x1};
    unsigned char makeformat[2] = {0x2, 0x0};
    unsigned char Ncomponent[4] = {32, 0x0, 0x0, 0x0};
    char make[32];
    unsigned char makeoffchar[4];
    unsigned char *offset;

    // MODEL
    unsigned char modeltag[2] = {0x10, 0x1};
    unsigned char modelformat[2] = {0x2, 0x0};
    unsigned char NcomponentModel[4] = {32, 0x0, 0x0, 0x0};
    char model[32];
    unsigned char modeloffchar[4];

    // ORIENTATION
    unsigned char orientationtag[2] = {0x12, 0x1};
    unsigned char orientationformat[2] = {0x3, 0x0};
    unsigned char NcomponentOrientation[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t Orientation[1];
    unsigned char Orient[4] = {0};

    // JPEG PROCESS
    unsigned char Processtag[2] = {0x00, 0x02};
    unsigned char Processformat[2] = {0x3, 0x0};
    unsigned char NcomponentProcess[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t Process[1];
    unsigned char Proc[4] = {0};

    // SOFTWARE
    unsigned char Versiontag[2] = {0x31, 0x1};
    unsigned char Versionformat[2] = {0x2, 0x0};
    unsigned char NcomponentVersion[4] = {32, 0x0, 0x0, 0x0};
    char Version[32];
    unsigned char Versionoffchar[4];

    // DATE/TIME
    unsigned char DateTimetag[2] = {0x32, 0x1};
    unsigned char DateTimeformat[2] = {0x2, 0x0};
    unsigned char NcomponentDateTime[4] = {20, 0, 0, 0};
    unsigned char DateTime[32];
    char DateTimeClose[1] = {0};
    unsigned char DateTimeoffchar[4];

    // COPYRIGHT
    unsigned char CopyRighttag[2] = {0x98, 0x82};
    unsigned char CopyRightformat[2] = {0x2, 0x0};
    unsigned char NcomponentCopyRight[4] = {32, 0x0, 0x0, 0x0};
    char CopyRight[32];
    unsigned char CopyRightoffchar[4];

    // SUBIFD
    unsigned char SubIFDOffsettag[2] = {0x69, 0x87};
    unsigned char SubIFDOffsetformat[2] = {0x4, 0x0};
    unsigned char NcomponentSubIFDOffset[4] = {0x1, 0x0, 0x0, 0x0};
    unsigned char SubIFDOffsetChar[4] = {0};

    // EXPOSURE TIME
    unsigned char ExposureTimetag[2] = {0x9A, 0x82};
    unsigned char ExposureTimeformat[2] = {0x5, 0x0};
    unsigned char NcomponentExposureTime[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t ExposureTimeNum[1];
    uint32_t ExposureTimeDen[1];

    unsigned char ExposureTimeoffchar[4];
    unsigned char ExposureTimeNumChar[4];
    unsigned char ExposureTimeDenChar[4];

    // FNUMBER
    unsigned char FNumbertag[2] = {0x9D, 0x82};
    unsigned char FNumberformat[2] = {0x5, 0x0};
    unsigned char NcomponentFNumber[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t FNumberNum[1];
    uint32_t FNumberDen[1];

    unsigned char FNumberoffchar[4];
    unsigned char FNumberNumChar[4];
    unsigned char FNumberDenChar[4];

    // EXPOSURE PROGRAM
    unsigned char ExposureProgramtag[2] = {0x22, 0x88};
    unsigned char ExposureProgramformat[2] = {0x3, 0x0};
    unsigned char NcomponentExposureProgram[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t ExposureProgram[1];
    unsigned char ExposureProgramChar[4] = {0};

    // ISO
    unsigned char ISOSpeedRatingstag[2] = {0x27, 0x88};
    unsigned char ISOSpeedRatingsformat[2] = {0x3, 0x0};
    unsigned char NcomponentISOSpeedRatings[4] = {0x2, 0x0, 0x0, 0x0};
    unsigned short ISOSpeedRatings[2];
    unsigned char ISOSpeedRatingsChar[4] = {0};

    // IMAGE
    unsigned char Brightnesstag[2] = {0x03, 0x92};
    unsigned char Brightnessformat[2] = {0xA, 0x0};
    unsigned char NcomponentBrightness[4] = {0x1, 0x0, 0x0, 0x0};
    int32_t BrightnessNum[1];
    int32_t BrightnessDen[1];

    unsigned char Brightnessoffchar[4];
    unsigned char BrightnessNumChar[4];
    unsigned char BrightnessDenChar[4];

    // EXPOSURE Bias
    unsigned char ExposureBiastag[2] = {0x04, 0x92};
    unsigned char ExposureBiasformat[2] = {0xA, 0x0};
    unsigned char NcomponentExposureBias[4] = {0x1, 0x0, 0x0, 0x0};
    int32_t ExposureBiasNum[1];
    int32_t ExposureBiasDen[1];

    unsigned char ExposureBiasoffchar[4];
    unsigned char ExposureBiasNumChar[4];
    unsigned char ExposureBiasDenChar[4];

    // SUBJECT DISTANCE
    unsigned char SubjectDistancetag[2] = {0x06, 0x92};
    unsigned char SubjectDistanceformat[2] = {0xA, 0x0};
    unsigned char NcomponentSubjectDistance[4] = {0x1, 0x0, 0x0, 0x0};
    int32_t SubjectDistanceNum[1];
    int32_t SubjectDistanceDen[1];

    unsigned char SubjectDistanceoffchar[4];
    unsigned char SubjectDistanceNumChar[4];
    unsigned char SubjectDistanceDenChar[4];

    // METERING MODE
    unsigned char MeteringModetag[2] = {0x07, 0x92};
    unsigned char MeteringModeformat[2] = {0x3, 0x0};
    unsigned char NcomponentMeteringMode[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t MeteringMode[1];
    unsigned char MeteringModeChar[4] = {0};

    // FLASH
    unsigned char Flashtag[2] = {0x09, 0x92};
    unsigned char Flashformat[2] = {0x3, 0x0};
    unsigned char NcomponentFlash[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t Flash[1] = {1};
    unsigned char FlashChar[4] = {0};

    // FOCAL LENGTH
    unsigned char FocalLengthtag[2] = {0x0A, 0x92};
    unsigned char FocalLengthformat[2] = {0x5, 0x0};
    unsigned char NcomponentFocalLength[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t FocalLengthNum[1];
    uint32_t FocalLengthDen[1];

    unsigned char FocalLengthoffchar[4];
    unsigned char FocalLengthNumChar[4];
    unsigned char FocalLengthDenChar[4];

    // WIDTH
    unsigned char Widthtag[2] = {0x02, 0xA0};
    unsigned char Widthformat[2] = {0x3, 0x0};
    unsigned char NcomponentWidth[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t Width[1];
    unsigned char WidthChar[4] = {0};

    // HEIGHT
    unsigned char Heighttag[2] = {0x03, 0xA0};
    unsigned char Heightformat[2] = {0x3, 0x0};
    unsigned char NcomponentHeight[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t Height[1];
    unsigned char HeightChar[4] = {0};

    // COLORSPACE
    unsigned char ColorSpacetag[2] = {0x01, 0xA0};
    unsigned char ColorSpaceformat[2] = {0x3, 0x0};
    unsigned char NcomponentColorSpace[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t ColorSpace[1];
    unsigned char ColorSpaceChar[4] = {0};

    // FocalPlaneXResolution
    unsigned char FocalPlaneXResolutiontag[2] = {0x0E, 0xA2};
    unsigned char FocalPlaneXResolutionformat[2] = {0x5, 0x0};
    unsigned char NcomponentFocalPlaneXResolution[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t FocalPlaneXResolutionNum[1];
    uint32_t FocalPlaneXResolutionDen[1];

    unsigned char FocalPlaneXResolutionoffchar[4];
    unsigned char FocalPlaneXResolutionNumChar[4];
    unsigned char FocalPlaneXResolutionDenChar[4];

    // FocalPlaneYResolution
    unsigned char FocalPlaneYResolutiontag[2] = {0x0F, 0xA2};
    unsigned char FocalPlaneYResolutionformat[2] = {0x5, 0x0};
    unsigned char NcomponentFocalPlaneYResolution[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t FocalPlaneYResolutionNum[1];
    uint32_t FocalPlaneYResolutionDen[1];

    unsigned char FocalPlaneYResolutionoffchar[4];
    unsigned char FocalPlaneYResolutionNumChar[4];
    unsigned char FocalPlaneYResolutionDenChar[4];

    // FocalPlaneResolutionUnit
    unsigned char FocalPlaneResolutionUnittag[2] = {0x10, 0xA2};
    unsigned char FocalPlaneResolutionUnitformat[2] = {0x3, 0x0};
    unsigned char NcomponentFocalPlaneResolutionUnit[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t FocalPlaneResolutionUnit[1];
    unsigned char FocalPlaneResolutionUnitChar[4] = {0};

    // BALANCE PROGRAM
    unsigned char WhiteBalancetag[2] = {0x07, 0x00};
    unsigned char WhiteBalanceformat[2] = {0x3, 0x0};
    unsigned char NcomponentWhiteBalance[4] = {0x1, 0x0, 0x0, 0x0};
    uint32_t WhiteBalance[1];
    unsigned char WhiteBalanceChar[4] = {0};

    // USER COMMENTS
    unsigned char UserCommentstag[2] = {0x86, 0x92};
    unsigned char UserCommentsformat[2] = {0x7, 0x0};
    unsigned char NcomponentUserComments[4] = {150, 0x0, 0x0, 0x0};
    unsigned char UserComments[150];
    unsigned char UserCommentsoffchar[4];
    // END OF THE VARIABLES

    ExifInitialCount = tempExif;
    // for APP1 Marker(2 byte) and length(2 byte)
    tempExif += 4;
    // write an exif header
    memcpy(tempExif, ExifHeader, 6);
    tempExif += 6;

    //write a tiff header
    memcpy(tempExif, TIFFHeader, 8);
    startoftiff = tempExif;
    tempExif += 8;
    //write no of entries in 1d0
    memcpy(tempExif, Nentries, 2);
    tempExif += 2;

    // ENTRY MAKE
    // write make tag
    memcpy(tempExif, maketag, 2);
    tempExif += 2;
    // write format
    memcpy(tempExif, makeformat, 2);
    tempExif += 2;
    // write no of component
    memcpy(tempExif, Ncomponent, 4);
    tempExif += 4;
    // write make
    memcpy(make, exifFileInfo->Make, 32);
    offset = (unsigned char *)0x200;
    santemp = (uint32_t)(offset);
    makeoffchar[0] = (unsigned char)santemp;
    makeoffchar[1] = (unsigned char)(santemp>>8);
    makeoffchar[2] = (unsigned char)(santemp>>16);
    makeoffchar[3] = (unsigned char)(santemp>>24);
    // write the make offset into the bitstream
    memcpy(tempExif, makeoffchar, 4);
    tempExif += 4;
    memcpy(startoftiff + santemp, make, 32);

    offset += 32;

    // ENTRY MODEL
    memcpy(tempExif, modeltag, 2);
    tempExif += 2;
    memcpy(tempExif, modelformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentModel, 4);
    tempExif += 4;
    memcpy(model, exifFileInfo->Model, 32);
    santemp = (uint32_t)(offset);
    modeloffchar[0]= (unsigned char)santemp;
    modeloffchar[1]= (unsigned char)(santemp>>8);
    modeloffchar[2]= (unsigned char)(santemp>>16);
    modeloffchar[3]= (unsigned char)(santemp>>24);
    // write the model offset into the bitstream
    memcpy(tempExif, modeloffchar, 4);
    tempExif += 4;
    memcpy(startoftiff + santemp, model, 32);

    offset+=32;

    // ENTRY ORIENTATION
    memcpy(tempExif, orientationtag, 2);
    tempExif += 2;
    memcpy(tempExif, orientationformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentOrientation, 4);
    tempExif += 4;
    Orientation[0] = exifFileInfo->Orientation;
    Orient[0] = (unsigned char)(Orientation[0]);
    Orient[1] = (unsigned char)(Orientation[0]>>8);
    Orient[2] = (unsigned char)(Orientation[0]>>16);
    Orient[3] = (unsigned char)(Orientation[0]>>24);
    memcpy(tempExif, Orient, 4);

    tempExif += 4;

    // ENTRY JPEG PROCESS
    memcpy(tempExif, Processtag, 2);
    tempExif += 2;
    memcpy(tempExif, Processformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentProcess, 4);
    tempExif += 4;
    Process[0] = exifFileInfo->Process;
    Proc[0] = (unsigned char) ( Process[0]);
    Proc[1] = (unsigned char) ( Process[0]>>8);
    Proc[2] = (unsigned char) ( Process[0]>>16);
    Proc[3] = (unsigned char) ( Process[0]>>24);
    memcpy(tempExif, Proc, 4);

    tempExif += 4;

    // ENTRY software
    memcpy(tempExif, Versiontag, 2);
    tempExif += 2;
    memcpy(tempExif, Versionformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentVersion, 4);
    tempExif += 4;
    santemp = (uint32_t)(offset);
    Versionoffchar[0]= (unsigned char)santemp;
    Versionoffchar[1]= (unsigned char)(santemp>>8);
    Versionoffchar[2]= (unsigned char)(santemp>>16);
    Versionoffchar[3]= (unsigned char)(santemp>>24);
    memcpy(tempExif, Versionoffchar, 4);
    tempExif += 4;
    memcpy(Version, exifFileInfo->Version, 32);
    memcpy(startoftiff+santemp, Version, 32);

    offset += 32;

    // ENTRY Date/Time
    memcpy(tempExif, DateTimetag, 2);
    tempExif += 2;
    memcpy(tempExif, DateTimeformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentDateTime, 4);
    tempExif += 4;
    memcpy(DateTime, exifFileInfo->DateTime, 20);
    santemp = (uint32_t)(offset);
    DateTimeoffchar[0] = (unsigned char)santemp;
    DateTimeoffchar[1] = (unsigned char)(santemp>>8);
    DateTimeoffchar[2] = (unsigned char)(santemp>>16);
    DateTimeoffchar[3] = (unsigned char)(santemp>>24);
    memcpy (tempExif, DateTimeoffchar, 4);
    tempExif += 4;
    memcpy (startoftiff+santemp, DateTime, 19);
    memcpy (startoftiff+santemp + 19, DateTimeClose, 1);

    offset += 32;

    //ENTRY COPYRIGHT INFO
    memcpy(tempExif, CopyRighttag, 2);
    tempExif += 2;
    memcpy(tempExif, CopyRightformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentCopyRight, 4);
    tempExif += 4;
    memcpy(CopyRight, exifFileInfo->CopyRight, 32);
    santemp = (uint32_t)(offset);
    CopyRightoffchar[0] = (unsigned char)santemp;
    CopyRightoffchar[1] = (unsigned char)(santemp>>8);
    CopyRightoffchar[2] = (unsigned char)(santemp>>16);
    CopyRightoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, CopyRightoffchar, 4);
    tempExif += 4;
    memcpy(startoftiff + santemp, CopyRight, 32);

    offset += 32;

    // ENTRY OFFSET TO THE SubIFD
    memcpy(tempExif, SubIFDOffsettag, 2);
    tempExif += 2;
    memcpy(tempExif, SubIFDOffsetformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentSubIFDOffset, 4);
    tempExif += 4;
    santemp = (int32_t)(tempExif - startoftiff + 8);
    SubIFDOffsetChar[0] = (unsigned char)(santemp);
    SubIFDOffsetChar[1] = (unsigned char)(santemp>>8);
    SubIFDOffsetChar[2] = (unsigned char)(santemp>>16);
    SubIFDOffsetChar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, SubIFDOffsetChar, 4);

    tempExif += 4;

    // since it was the last directory entry, so next 4 bytes contains an offset to the IFD1.
    // since we dont know the offset lets put 0x0000 as an offset, later when get to know the
    // actual offset we will revisit here and put the actual offset.
    santemp = 0x0000;
    SubIFDOffsetChar[0] = (unsigned char)(santemp);
    SubIFDOffsetChar[1] = (unsigned char)(santemp>>8);
    SubIFDOffsetChar[2] = (unsigned char)(santemp>>16);
    SubIFDOffsetChar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, SubIFDOffsetChar, 4);
    tempExif += 4;

    //EXIF SUBIFD STARTS HERE
    //write no of entries in SubIFD
    memcpy(tempExif, SubIFDNentries, 2);
    tempExif += 2;

    // ENTRY EXPOSURE TIME
    memcpy(tempExif, ExposureTimetag, 2);
    tempExif += 2;
    memcpy(tempExif, ExposureTimeformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentExposureTime, 4);
    tempExif += 4;
    santemp = (uint32_t)(offset);
    ExposureTimeoffchar[0] = (unsigned char)santemp;
    ExposureTimeoffchar[1] = (unsigned char)(santemp>>8);
    ExposureTimeoffchar[2] = (unsigned char)(santemp>>16);
    ExposureTimeoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, ExposureTimeoffchar, 4);
    tempExif += 4 ;
    ExposureTimeNum[0] = exifFileInfo->ExposureTimeNum;
    ExposureTimeDen[0] = exifFileInfo->ExposureTimeDen;
    ExposureTimeNumChar[0] = (unsigned char) ExposureTimeNum[0];
    ExposureTimeNumChar[1] = (unsigned char)(ExposureTimeNum[0]>>8);
    ExposureTimeNumChar[2] = (unsigned char)(ExposureTimeNum[0]>>16);
    ExposureTimeNumChar[3] = (unsigned char)(ExposureTimeNum[0]>>24);

    ExposureTimeDenChar[0] = (unsigned char)ExposureTimeDen[0];
    ExposureTimeDenChar[1] = (unsigned char)(ExposureTimeDen[0]>>8);
    ExposureTimeDenChar[2] = (unsigned char)(ExposureTimeDen[0]>>16);
    ExposureTimeDenChar[3] = (unsigned char)(ExposureTimeDen[0]>>24);

    memcpy(startoftiff + santemp, ExposureTimeNumChar, 4);
    memcpy(startoftiff + santemp+4, ExposureTimeDenChar, 4);

    offset += 32;

    // ENTRY F NUMBER
    memcpy(tempExif, FNumbertag, 2);
    tempExif += 2;
    memcpy(tempExif, FNumberformat, 2);
    tempExif += 2 ;
    memcpy(tempExif, NcomponentFNumber, 4);
    tempExif += 4;
    santemp = (uint32_t)(offset);
    FNumberoffchar[0] = (unsigned char)santemp;
    FNumberoffchar[1] = (unsigned char)(santemp>>8);
    FNumberoffchar[2] = (unsigned char)(santemp>>16);
    FNumberoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, FNumberoffchar, 4);
    tempExif += 4;

    FNumberNum[0] = exifFileInfo->FNumberNum;
    FNumberDen[0] = exifFileInfo->FNumberDen;

    FNumberNumChar[0] = (unsigned char)FNumberNum[0];
    FNumberNumChar[1] = (unsigned char)(FNumberNum[0]>>8);
    FNumberNumChar[2] = (unsigned char)(FNumberNum[0]>>16);
    FNumberNumChar[3] = (unsigned char)(FNumberNum[0]>>24);

    FNumberDenChar[0] = (unsigned char)FNumberDen[0];
    FNumberDenChar[1] = (unsigned char)(FNumberDen[0]>>8);
    FNumberDenChar[2] = (unsigned char)(FNumberDen[0]>>16);
    FNumberDenChar[3] = (unsigned char)(FNumberDen[0]>>24);

    memcpy(startoftiff + santemp, FNumberNumChar, 4);
    memcpy(startoftiff + santemp+4, FNumberDenChar, 4);

    offset += 32;

    // ENTRY EXPOSURE PROGRAM
    memcpy(tempExif, ExposureProgramtag, 2);
    tempExif += 2;
    memcpy(tempExif, ExposureProgramformat, 2);
    tempExif += 2;
    memcpy (tempExif, NcomponentExposureProgram, 4);
    tempExif += 4;
    ExposureProgram[0] = exifFileInfo->ExposureProgram;
    ExposureProgramChar[0] = (unsigned char)(ExposureProgram[0]);
    ExposureProgramChar[1] = (unsigned char)(ExposureProgram[0]>>8);
    ExposureProgramChar[2] = (unsigned char)(ExposureProgram[0]>>16);
    ExposureProgramChar[3] = (unsigned char)(ExposureProgram[0]>>24);

    memcpy(tempExif, ExposureProgramChar, 4);

    tempExif += 4;

    // ENTRY ISOSpeedRatings
    memcpy(tempExif, ISOSpeedRatingstag, 2);
    tempExif += 2;
    memcpy(tempExif, ISOSpeedRatingsformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentISOSpeedRatings, 4);
    tempExif += 4;
    ISOSpeedRatings[0] = 1;
    ISOSpeedRatings[1] = 2;
    ISOSpeedRatingsChar[0] = (unsigned char)(ISOSpeedRatings[0]);
    ISOSpeedRatingsChar[1] = (unsigned char)(ISOSpeedRatings[0]>>8);
    ISOSpeedRatingsChar[2] = (unsigned char)(ISOSpeedRatings[1]);
    ISOSpeedRatingsChar[3] = (unsigned char)(ISOSpeedRatings[1]>>8);

    memcpy(tempExif, ISOSpeedRatingsChar, 4);
    tempExif += 4 ;

    // BRIGHTNESS OF THE IMAGE
    memcpy(tempExif, Brightnesstag, 2);
    tempExif += 2;
    memcpy(tempExif, Brightnessformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentBrightness, 4);
    tempExif += 4;
    santemp = (uint32_t)(offset);
    Brightnessoffchar[0] = (unsigned char)santemp;
    Brightnessoffchar[1] = (unsigned char)(santemp>>8);
    Brightnessoffchar[2] = (unsigned char)(santemp>>16);
    Brightnessoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, Brightnessoffchar, 4);
    tempExif += 4 ;

    BrightnessNum[0] = exifFileInfo->BrightnessNum;
    BrightnessDen[0] = exifFileInfo->BrightnessDen;

    BrightnessNumChar[0] = (unsigned char)BrightnessNum[0];
    BrightnessNumChar[1] = (unsigned char)(BrightnessNum[0]>>8);
    BrightnessNumChar[2] = (unsigned char)(BrightnessNum[0]>>16);
    BrightnessNumChar[3] = (unsigned char)(BrightnessNum[0]>>24);

    BrightnessDenChar[0] = (unsigned char)BrightnessDen[0];
    BrightnessDenChar[1] = (unsigned char)(BrightnessDen[0]>>8);
    BrightnessDenChar[2] = (unsigned char)(BrightnessDen[0]>>16);
    BrightnessDenChar[3] = (unsigned char)(BrightnessDen[0]>>24);

    memcpy(startoftiff + santemp, BrightnessNumChar, 4);
    memcpy(startoftiff+santemp + 4, BrightnessDenChar, 4);

    offset += 48;

    // EXPOSURE BIAS OF THE IMAGE
    memcpy(tempExif, ExposureBiastag, 2);
    tempExif += 2 ;
    memcpy(tempExif, ExposureBiasformat, 2);
    tempExif += 2 ;
    memcpy(tempExif, NcomponentExposureBias, 4);
    tempExif += 4 ;

    santemp = (uint32_t)(offset);
    ExposureBiasoffchar[0] = (unsigned char)santemp;
    ExposureBiasoffchar[1] = (unsigned char)(santemp>>8);
    ExposureBiasoffchar[2] = (unsigned char)(santemp>>16);
    ExposureBiasoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, ExposureBiasoffchar, 4);
    tempExif += 4;
    ExposureBiasNum[0] = exifFileInfo->ExposureBiasNum;
    ExposureBiasDen[0] = exifFileInfo->ExposureBiasDen;
    ExposureBiasNumChar[0] = (unsigned char)ExposureBiasNum[0];
    ExposureBiasNumChar[1] = (unsigned char)(ExposureBiasNum[0]>>8);
    ExposureBiasNumChar[2] = (unsigned char)(ExposureBiasNum[0]>>16);
    ExposureBiasNumChar[3] = (unsigned char)(ExposureBiasNum[0]>>24);

    ExposureBiasDenChar[0] = (unsigned char)ExposureBiasDen[0];
    ExposureBiasDenChar[1] = (unsigned char)(ExposureBiasDen[0]>>8);
    ExposureBiasDenChar[2] = (unsigned char)(ExposureBiasDen[0]>>16);
    ExposureBiasDenChar[3] = (unsigned char)(ExposureBiasDen[0]>>24);

    memcpy(startoftiff + santemp, ExposureBiasNumChar, 4);
    memcpy(startoftiff + santemp + 4, ExposureBiasDenChar, 4);

    offset += 48;

    // SUBJECT DISTANCE
    memcpy(tempExif, SubjectDistancetag, 2);
    tempExif += 2;
    memcpy(tempExif, SubjectDistanceformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentSubjectDistance, 4);
    tempExif += 4;
    santemp = (uint32_t)(offset);
    SubjectDistanceoffchar[0] = (unsigned char)santemp;
    SubjectDistanceoffchar[1] = (unsigned char)(santemp>>8);
    SubjectDistanceoffchar[2] = (unsigned char)(santemp>>16);
    SubjectDistanceoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, SubjectDistanceoffchar, 4);
    tempExif += 4;
    SubjectDistanceNum[0] = exifFileInfo->SubjectDistanceNum;
    SubjectDistanceDen[0] = exifFileInfo->SubjectDistanceDen;
    SubjectDistanceNumChar[0] = (unsigned char )SubjectDistanceNum[0];
    SubjectDistanceNumChar[1] = (unsigned char )(SubjectDistanceNum[0]>>8);
    SubjectDistanceNumChar[2] = (unsigned char )(SubjectDistanceNum[0]>>16);
    SubjectDistanceNumChar[3] = (unsigned char )(SubjectDistanceNum[0]>>24);

    SubjectDistanceDenChar[0]= (unsigned char)SubjectDistanceDen[0];
    SubjectDistanceDenChar[1]= (unsigned char)(SubjectDistanceDen[0]>>8);
    SubjectDistanceDenChar[2]= (unsigned char)(SubjectDistanceDen[0]>>16);
    SubjectDistanceDenChar[3] = (unsigned char)(SubjectDistanceDen[0]>>24);

    memcpy(startoftiff + santemp, SubjectDistanceNumChar, 4);
    memcpy(startoftiff + santemp + 4, SubjectDistanceDenChar, 4);

    offset += 48;

    // METERING MODE
    memcpy(tempExif, MeteringModetag, 2);
    tempExif += 2;
    memcpy(tempExif, MeteringModeformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentMeteringMode, 4);
    tempExif += 4;
    MeteringMode[0] = exifFileInfo->MeteringMode;
    MeteringModeChar[0] = (unsigned char)(MeteringMode[0]);
    MeteringModeChar[1] = (unsigned char)(MeteringMode[0]>>8);
    MeteringModeChar[2] = (unsigned char)(MeteringMode[0]>>16);
    MeteringModeChar[3] = (unsigned char)(MeteringMode[0]>>24);

    memcpy(tempExif, MeteringModeChar, 4);

    tempExif += 4;

    //mENTRY FLASH
    memcpy(tempExif, Flashtag, 2);
    tempExif += 2;
    memcpy(tempExif, Flashformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentFlash, 4);
    tempExif += 4;
    Flash[0] = exifFileInfo->Flash;
    FlashChar[0] = (unsigned char)(Flash[0]);
    FlashChar[1] = (unsigned char)(Flash[0]>>8);
    FlashChar[2] = (unsigned char)(Flash[0]>>16);
    FlashChar[3] = (unsigned char)(Flash[0]>>24);
    memcpy(tempExif, FlashChar, 4);

    tempExif += 4;

    // FOCAL LENGTH
    memcpy(tempExif, FocalLengthtag, 2);
    tempExif += 2 ;
    memcpy(tempExif, FocalLengthformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentFocalLength, 4);

    tempExif += 4;

    santemp = (uint32_t)(offset);
    FocalLengthoffchar[0] = (unsigned char)santemp;
    FocalLengthoffchar[1] = (unsigned char)(santemp>>8);
    FocalLengthoffchar[2] = (unsigned char)(santemp>>16);
    FocalLengthoffchar[3] = (unsigned char)(santemp>>24);

    memcpy(tempExif, FocalLengthoffchar, 4);

    tempExif += 4;
    FocalLengthNum[0] = exifFileInfo->FocalLengthNum;
    FocalLengthDen[0] = exifFileInfo->FocalLengthDen;
    FocalLengthNumChar[0] = (unsigned char)FocalLengthNum[0];
    FocalLengthNumChar[1] = (unsigned char)(FocalLengthNum[0]>>8);
    FocalLengthNumChar[2] = (unsigned char)(FocalLengthNum[0]>>16);
    FocalLengthNumChar[3] = (unsigned char)(FocalLengthNum[0]>>24);

    FocalLengthDenChar[0] = (unsigned char)FocalLengthDen[0];
    FocalLengthDenChar[1] = (unsigned char)(FocalLengthDen[0]>>8);
    FocalLengthDenChar[2] = (unsigned char)(FocalLengthDen[0]>>16);
    FocalLengthDenChar[3] = (unsigned char)(FocalLengthDen[0]>>24);

    memcpy(startoftiff + santemp, FocalLengthNumChar, 4);
    memcpy(startoftiff + santemp+4, FocalLengthDenChar, 4);

    offset += 48;

    // ENTRY Width
    memcpy(tempExif, Widthtag, 2);
    tempExif += 2;
    memcpy(tempExif, Widthformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentWidth, 4);
    tempExif += 4;
    Width[0] = exifFileInfo->Width;
    WidthChar[0] = (unsigned char)(Width[0]);
    WidthChar[1] = (unsigned char)(Width[0]>>8);
    WidthChar[2] = (unsigned char)(Width[0]>>16);
    WidthChar[3] = (unsigned char)(Width[0]>>24);

    memcpy(tempExif, WidthChar, 4);

    tempExif += 4;

    // ENTRY Height
    memcpy(tempExif, Heighttag, 2);
    tempExif += 2;
    memcpy(tempExif, Heightformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentHeight, 4);
    tempExif += 4;
    Height[0] = exifFileInfo->Height;
    HeightChar[0] = (unsigned char)(Height[0]);
    HeightChar[1] = (unsigned char)(Height[0]>>8);
    HeightChar[2] = (unsigned char)(Height[0]>>16);
    HeightChar[3] = (unsigned char)(Height[0]>>24);

    memcpy(tempExif, HeightChar, 4);

    tempExif += 4 ;

    // ENTRY COLORSPACE
    memcpy(tempExif, ColorSpacetag, 2);
    tempExif += 2;
    memcpy(tempExif, ColorSpaceformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentColorSpace, 4);
    tempExif += 4;
    ColorSpace [0]= exifFileInfo->ColorSpace;
    ColorSpaceChar[0] = (unsigned char)(ColorSpace[0]);
    ColorSpaceChar[1] = (unsigned char)(ColorSpace[0]>>8);
    ColorSpaceChar[2] = (unsigned char)(ColorSpace[0]>>16);
    ColorSpaceChar[3] = (unsigned char)(ColorSpace[0]>>24);

    memcpy(tempExif, ColorSpaceChar, 4);

    tempExif += 4;

    // ENTRY FocalPlaneXResolution
    memcpy(tempExif, FocalPlaneXResolutiontag, 2);
    tempExif += 2;
    memcpy(tempExif, FocalPlaneXResolutionformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentFocalPlaneXResolution, 4);
    tempExif += 4;

    santemp = (uint32_t)(offset);
    FocalPlaneXResolutionoffchar[0] = (unsigned char)santemp;
    FocalPlaneXResolutionoffchar[1] = (unsigned char)(santemp>>8);
    FocalPlaneXResolutionoffchar[2] = (unsigned char)(santemp>>16);
    FocalPlaneXResolutionoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, FocalPlaneXResolutionoffchar, 4);
    tempExif += 4;
    FocalPlaneXResolutionNum[0] = exifFileInfo->FocalPlaneXResolutionNum;
    FocalPlaneXResolutionDen[0] = exifFileInfo->FocalPlaneXResolutionDen;

    FocalPlaneXResolutionNumChar[0] = (unsigned char)FocalPlaneXResolutionNum[0];
    FocalPlaneXResolutionNumChar[1] = (unsigned char)(FocalPlaneXResolutionNum[0]>>8);
    FocalPlaneXResolutionNumChar[2] = (unsigned char)(FocalPlaneXResolutionNum[0]>>16);
    FocalPlaneXResolutionNumChar[3] = (unsigned char)(FocalPlaneXResolutionNum[0]>>24);

    FocalPlaneXResolutionDenChar[0] = (unsigned char)FocalPlaneXResolutionDen[0];
    FocalPlaneXResolutionDenChar[1] = (unsigned char)(FocalPlaneXResolutionDen[0]>>8);
    FocalPlaneXResolutionDenChar[2] = (unsigned char)(FocalPlaneXResolutionDen[0]>>16);
    FocalPlaneXResolutionDenChar[3] = (unsigned char)(FocalPlaneXResolutionDen[0]>>24);

    memcpy(startoftiff+santemp, FocalPlaneXResolutionNumChar, 4);
    memcpy(startoftiff+santemp+4, FocalPlaneXResolutionDenChar, 4);

    offset += 48;

    // ENTRY FocalPlaneYResolution
    memcpy ( tempExif, FocalPlaneYResolutiontag, 2);
    tempExif += 2;
    memcpy(tempExif, FocalPlaneYResolutionformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentFocalPlaneYResolution, 4);
    tempExif += 4;

    santemp = (uint32_t)(offset);
    FocalPlaneYResolutionoffchar[0] = (unsigned char)santemp;
    FocalPlaneYResolutionoffchar[1] = (unsigned char)(santemp>>8);
    FocalPlaneYResolutionoffchar[2] = (unsigned char)(santemp>>16);
    FocalPlaneYResolutionoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, FocalPlaneYResolutionoffchar, 4);
    tempExif += 4;
    FocalPlaneYResolutionNum[0] = exifFileInfo->FocalPlaneYResolutionNum;
    FocalPlaneYResolutionDen[0] = exifFileInfo->FocalPlaneYResolutionDen;

    FocalPlaneYResolutionNumChar[0] = (unsigned char)FocalPlaneYResolutionNum[0];
    FocalPlaneYResolutionNumChar[1] = (unsigned char)(FocalPlaneYResolutionNum[0]>>8);
    FocalPlaneYResolutionNumChar[2] = (unsigned char)(FocalPlaneYResolutionNum[0]>>16);
    FocalPlaneYResolutionNumChar[3] = (unsigned char)(FocalPlaneYResolutionNum[0]>>24);

    FocalPlaneYResolutionDenChar[0] = (unsigned char)FocalPlaneYResolutionDen[0];
    FocalPlaneYResolutionDenChar[1] = (unsigned char)(FocalPlaneYResolutionDen[0]>>8);
    FocalPlaneYResolutionDenChar[2] = (unsigned char)(FocalPlaneYResolutionDen[0]>>16);
    FocalPlaneYResolutionDenChar[3] = (unsigned char)(FocalPlaneYResolutionDen[0]>>24);

    memcpy(startoftiff+santemp, FocalPlaneYResolutionNumChar, 4);
    memcpy(startoftiff+santemp + 4, FocalPlaneYResolutionDenChar, 4);

    offset += 48;

    // ENTRY FocalPlaneResolutionUnit
    memcpy(tempExif, FocalPlaneResolutionUnittag, 2);
    tempExif += 2 ;
    memcpy(tempExif, FocalPlaneResolutionUnitformat, 2);
    tempExif += 2 ;
    memcpy(tempExif, NcomponentFocalPlaneResolutionUnit, 4);
    tempExif += 4;
    FocalPlaneResolutionUnit[0] = exifFileInfo->FocalPlaneResolutionUnit;
    FocalPlaneResolutionUnitChar[0] = (unsigned char)(FocalPlaneResolutionUnit[0]);
    FocalPlaneResolutionUnitChar[1] = (unsigned char)(FocalPlaneResolutionUnit[0]>>8);
    FocalPlaneResolutionUnitChar[2] = (unsigned char)(FocalPlaneResolutionUnit[0]>>16);
    FocalPlaneResolutionUnitChar[3] = (unsigned char)(FocalPlaneResolutionUnit[0]>>24);

    memcpy(tempExif, FocalPlaneResolutionUnitChar, 4);

    tempExif += 4 ;

    // ENTRY UserComments
    memcpy(tempExif, UserCommentstag, 2);
    tempExif += 2;
    memcpy(tempExif, UserCommentsformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentUserComments, 4);
    tempExif += 4;
    memcpy(UserComments, exifFileInfo->UserComments,150);
    santemp = (int32_t) (offset);
    UserCommentsoffchar[0] = (unsigned char)santemp;
    UserCommentsoffchar[1] = (unsigned char)(santemp>>8);
    UserCommentsoffchar[2] = (unsigned char)(santemp>>16);
    UserCommentsoffchar[3] = (unsigned char)(santemp>>24);
    memcpy(tempExif, UserCommentsoffchar, 4);
    tempExif += 4;
    memcpy(startoftiff+santemp, UserComments, 128);

    offset += 128;

    // ENTRY WHITE BALANCE
    memcpy(tempExif, WhiteBalancetag, 2);
    tempExif += 2;
    memcpy(tempExif, WhiteBalanceformat, 2);
    tempExif += 2;
    memcpy(tempExif, NcomponentWhiteBalance, 4);
    tempExif += 4;
    WhiteBalance[0] = exifFileInfo->WhiteBalance;
    WhiteBalanceChar[0] = (unsigned char)(WhiteBalance[0]);
    WhiteBalanceChar[1] = (unsigned char)(WhiteBalance[0]>>8);
    WhiteBalanceChar[2] = (unsigned char)(WhiteBalance[0]>>16);
    WhiteBalanceChar[3] = (unsigned char)(WhiteBalance[0]>>24);

    memcpy(tempExif, WhiteBalanceChar, 4);

    tempExif += 4;

    // WRITE END OF ENTRY FLAG
    memcpy(tempExif, EndOfEntry, 4);
    tempExif += 4;
    santemp = (uint32_t)(offset);

    // ENTRIES ARE OVER
    ExifSize = (santemp) + 8;
    ExifLen[1] = (unsigned char)(ExifSize);
    ExifLen[0] = (unsigned char)(ExifSize>>8);

    if (ExifSize > EXIF_FILE_SIZE + MAX_FILE_THUMB_SIZE - 2 || ExifSize < 0) {
        CVI_LOGE("makeExifFile  Invalid Exif size");
        tempExif = NULL;
        *totalLen = 0;
        return -1;
    }

    tempExif = ExifInitialCount;
    memcpy(tempExif, APP1Marker, 2);
    tempExif += 2;
    memcpy(tempExif, ExifLen, 2);
    *totalLen = ExifSize + 2;
    CVI_LOGD("CviMakeExifFile  totalLen : %d", *totalLen);
    return -1;
}

void CVI_EXIF_MakeExifParam(CVI_EXIF_FILE_INFO_S *exifFileInfo)
{
    if (!exifFileInfo->parame) {
        strcpy(exifFileInfo->Make, "CVITEK");
        strcpy(exifFileInfo->Model, "CVITEK00.0.1");
        strcpy(exifFileInfo->CopyRight, "Cvitek@All Rights Reserved");
        strcpy(exifFileInfo->Version, "CvitekCvrDV");
        exifFileInfo->Height = 2560;
        exifFileInfo->Width = 1440;
        exifFileInfo->Orientation = 1;
        exifFileInfo->ColorSpace = 1;
        exifFileInfo->Process = 1;
        exifFileInfo->Flash = 0;
        exifFileInfo->FocalLengthNum = 1;
        exifFileInfo->FocalLengthDen = 4;
        exifFileInfo->ExposureTimeNum = 1;
        exifFileInfo->ExposureTimeDen = 20;
        exifFileInfo->FNumberNum = 1;
        exifFileInfo->FNumberDen = 35;
        exifFileInfo->ApertureFNumber = 1;
        exifFileInfo->SubjectDistanceNum = -20;
        exifFileInfo->SubjectDistanceDen = -7;
        exifFileInfo->CCDWidth = 1;
        exifFileInfo->ExposureBiasNum = -16;
        exifFileInfo->ExposureBiasDen = -2;
        exifFileInfo->WhiteBalance = 6;
        exifFileInfo->MeteringMode = 3;
        exifFileInfo->ExposureProgram = 1;
        exifFileInfo->ISOSpeedRatings[0] = 1;
        exifFileInfo->ISOSpeedRatings[1] = 2;
        exifFileInfo->FocalPlaneXResolutionNum = 65;
        exifFileInfo->FocalPlaneXResolutionDen = 66;
        exifFileInfo->FocalPlaneYResolutionNum = 70;
        exifFileInfo->FocalPlaneYResolutionDen = 71;
        exifFileInfo->FocalPlaneResolutionUnit = 3;
        exifFileInfo->XResolutionNum = 48;
        exifFileInfo->XResolutionDen = 20;
        exifFileInfo->YResolutionNum = 48;
        exifFileInfo->YResolutionDen = 20;
        exifFileInfo->RUnit = 2;
        exifFileInfo->BrightnessNum = -7;
        exifFileInfo->BrightnessDen = 1;
        strcpy(exifFileInfo->UserComments, "Usercomments");
    }
}

int32_t CVI_EXIF_MakeNewSatJpgFromBuf(const char *src_file, const char *dest_file, char *buf, int32_t size)
{
    int32_t src_fd = open(src_file, O_RDONLY);
    if (-1 == src_fd) {
        CVI_LOGE("open file failed %s", src_file);
        return -1;
    }

    int32_t src_size = -1;
    struct stat _stat;
    int32_t value = fstat(src_fd, &_stat);
    if (0 == value) {
        src_size = _stat.st_size;
        CVI_LOGD("get file size %d", src_size);
    } else {
        CVI_LOGE("get file size failed %s", src_file);
        close(src_fd);
        return -1;
    }

    int32_t dest_fd = open(dest_file, O_RDWR | O_CREAT);
    if (-1 == dest_fd) {
        CVI_LOGE("open file failed %s", dest_file);
        close(src_fd);
        return -1;
    }

    char *buf_dest = malloc(src_size);
    int32_t read_size = read( src_fd, buf_dest, src_size );
    close(src_fd);
    if (read_size != src_size) {
        CVI_LOGE("read file failed %s", dest_file);
        free(buf_dest);
        close(dest_fd);
        return -1;
    }
    int32_t write_size = write(dest_fd, buf_dest, src_size);
    CVI_LOGD("read file write_size:%d  src_size:%d", write_size, src_size);
    free(buf_dest);
    if (write_size != src_size) {
        CVI_LOGE("open file failed %s", dest_file);
        close(dest_fd);
        return -1;
    }
    lseek(dest_fd, 0, SEEK_SET);

    char *buf_tail = malloc(src_size);

    unsigned char buf_data[4];
    read(dest_fd, buf_data, 2);
    if (buf_data[0] != 0xFF || buf_data[1] != 0xD8) {
        CVI_LOGE("JPEG file error %s", dest_file);
        free(buf_tail);
        close(dest_fd);
        return -1;
    }
    lseek(dest_fd, 0, SEEK_SET);
    while(1) {
        read(dest_fd, buf_data, 4);
        if ((buf_data[0] == 0xFF) && (buf_data[1] == 0xD8)) {
            lseek(dest_fd, -2, SEEK_CUR);
            int32_t _tail_size = read(dest_fd, buf_tail, write_size);
            if (_tail_size > 0) {
                lseek(dest_fd, -_tail_size, SEEK_CUR);
                write(dest_fd, buf, size);
                write(dest_fd, buf_tail, _tail_size);
                CVI_LOGD("After 0xFF 0xD8 has makeEif data: %d Byte, Src data: %d Byte, Total data: %d Byte", size, _tail_size, size+_tail_size);
                break;
            }
        } else {
            uint32_t app_size = (buf_data[2]<<8) + buf_data[3];
            int32_t offset = app_size - 2;
            if (-1 == lseek(dest_fd, offset, SEEK_CUR)) {
                CVI_LOGE("file seek erro");
                free(buf_tail);
                close(dest_fd);
                return -1;
            }
        }
    }

    free(buf_tail);
    close(dest_fd);
    return 0;
}