#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3.h"

void encodedprintf(id3buf str, int encoding){
    if(str == NULL){
        return;
    }

    if(encoding == ISO_8859_1 || encoding == UTF8){
        printf("%s",str);
    }
    
    if(encoding == UTF16){
        for(int i = 0; i < strlenUTF16(str); i++){
            if(!(str[i] == 0xff || str[i] == 0xfe || str[i] == 0x00)){
                printf("%c",str[i]);
            }
            
        }
    }

    if(encoding == UTF16BE){
        for(int i = 0; i < strlenUTF16BE(str); i++){
            if(!(str[i] == 0xff || str[i] == 0xfe || str[i] == 0x00)){
                printf("%c",str[i]);
            }
        }
    }
}

void metadataPrint(Id3Metadata *data){
    if(hasId3v1(data)){

        printf("ID3V1 tag information\n");
        Id3v1Tag *tag = data->version1;

        printf("artist | [%s]\n",tag->artist);
        printf("album  | [%s]\n",tag->albumTitle);
        printf("comment| [%s]\n",tag->comment);
        printf("genre  | [%s]\n",genreFromTable(tag->genre));
        printf("title  | [%s]\n",tag->title);
        printf("track  | [%d]\n",tag->trackNumber);
        printf("year   | [%d]\n",tag->year);
    }

    
    if(hasId3v2(data)){
        printf("ID3V2 tag information\n");

        Id3v2Tag *tag = data->version2;

        if(tag->header == NULL){
            printf("[*]no header\n");
        }else{
            printf("ver. %d\n",id3v2GetVersion(tag));
            printf("flags. %d%d%d%d\n",id3v2GetUnsynchronizedIndicator(tag), 
                                    id3v2GetExtendedIndicator(tag), 
                                    id3v2GetExperimentalIndicator(tag), 
                                    id3v2GetFooterIndicator(tag));
            printf("tag size. %ld\n",id3v2GetTagSize(tag));

            if(id3v2GetExtendedIndicator(tag) == true){
                printf("ext size. %d\n",tag->header->extendedHeader->size);
                printf("padding. %d\n",tag->header->extendedHeader->padding);
                printf("crc. %s\n",(char *)id3v2GetCrc(tag));
                printf("update. %d\n",tag->header->extendedHeader->update);
                printf("tag size restriction %x\n",id3v2GetTagSizeRestriction(tag));
                printf("tag encoding restriction %x\n",id3v2GetEncodingRestriction(tag));
                printf("text size restriction %x\n",id3v2GetTextSizeRestriction(tag));
                printf("image encoding restriction %x\n",id3v2GetEncodingRestriction(tag));
                printf("image size restriction %x\n",id3v2GetTagSizeRestriction(tag));
                
            }else{
                printf("[*]no extended header\n");
            }

        }
        
        if(tag->frames == NULL){
            printf("[*]no frames\n");

        }else{
            Id3v2Frame *currFrame = NULL;
            
            while((currFrame = id3v2IterTag(tag)) != NULL){

                //header info
                printf("%s|",id3v2GetFrameStrID(currFrame));
                printf("FS:%ld HS:%ld ID:%d|",id3v2GetFrameSize(currFrame),
                                            id3v2GetFrameHeaderSize(currFrame),
                                            id3v2GetFrameID(currFrame));

                //print flag content
                printf("F:%d%d%d%d C:%ld E:%x G:%x|",id3v2GetFrameAlterPreservationIndicator(currFrame),
                                                    id3v2GetFrameFileAlterPreservationIndicator(currFrame),
                                                    id3v2GetFrameReadOnlyIndicator(currFrame),
                                                    id3v2GetFrameUnsynchronizationIndicator(currFrame),
                                                    id3v2GetFrameDataLengthSize(currFrame),
                                                    id3v2GetFrameEncryptionMethod(currFrame),
                                                    id3v2GetFrameGroup(currFrame));
                
                int encoding = id3v2GetEncoding(currFrame);
                
                switch(encoding){
                    case ISO_8859_1:
                        printf("ISO_8859_1|");
                        break;
                    case UTF16:
                        printf("UTF16|");
                        break;
                    case UTF16BE:
                        printf("UTF16BE|");
                        break;
                    case UTF8:
                        printf("UTF8|");
                    default:
                        printf("None|");
                }
                
                if(id3v2GetFrameStrID(currFrame)[0] == 'T'){
                    
                        id3buf desc = id3v2GetDescription(currFrame);
                        id3buf value = id3v2GetTextValue(currFrame);
                        
                        printf("desc:[");
                        encodedprintf(desc, encoding);
                        printf("] ");
                        printf("value:[");
                        encodedprintf(value, encoding);
                        printf("]");

                        if(desc != NULL){
                            free(desc);
                        }
                        if(value != NULL){
                            free(value);
                        }
                
                }else if(id3v2GetFrameStrID(currFrame)[0] == 'W' && id3v2GetFrameID(currFrame) != WCOM){

                        id3buf desc = id3v2GetDescription(currFrame);
                        id3buf url = id3v2GetURLValue(currFrame);
                        
                        printf("desc:[");
                        encodedprintf(desc, encoding);
                        printf("] ");
                        printf("url:[%s]",url);

                        if(desc != NULL){
                            free(desc);
                        }
                        if(url != NULL){
                            free(url);
                        }     

                }else if(id3v2GetFrameID(currFrame) == IPL || id3v2GetFrameID(currFrame) == IPLS){
                    
                    id3buf people = id3v2GetInvolvedPeopleListValue(currFrame);
                    
                    printf("people:[");
                    encodedprintf(people, encoding);
                    printf("] ");

                    if(people != NULL){
                        free(people);
                    }  

                }else if(id3v2GetFrameID(currFrame) == MCI || id3v2GetFrameID(currFrame) == MCDI){

                    id3buf cdtoc = id3v2GetCDIDValue(currFrame);
                    printf("cdtoc[%s]",cdtoc);

                    if(cdtoc != NULL){
                        free(cdtoc);
                    }

                }else if(id3v2GetFrameID(currFrame) == ETC || id3v2GetFrameID(currFrame) == ETCO){

                    id3byte type = 0x00;
                    int time = 0;

                    printf("format:[%x] ",id3v2GetTimeStampFormat(currFrame));

                    printf("types:[");
                    while((type = id3v2GetEventTimeCodeType(currFrame)) != 0x00){
                        printf("[%x]",type);
                    }
                    printf("] ");

                    id3v2ResetEventTimeCodeIter(currFrame);

                    printf("stamps:[");
                    while((time = id3v2GetEventTimeCodeTimeStamp(currFrame)) != -1){
                        printf("[%d]",time);
                    }
                    printf("]");

                    id3v2ResetEventTimeCodeIter(currFrame);

                }else if(id3v2GetFrameID(currFrame) == ULT || id3v2GetFrameID(currFrame) == USLT){

                    id3buf language = id3v2GetLanguage(currFrame);
                    id3buf description = id3v2GetDescription(currFrame);
                    id3buf lyrics = id3v2GetUnsynchronizedLyrics(currFrame);

                    if(language != NULL){
                        printf("language:[%s] ",language);
                        free(language);
                    }

                    if(description != NULL){
                        printf("desc:[");
                        encodedprintf(description, encoding);
                        printf("] ");
                        free(description);
                    }

                    if(lyrics != NULL){
                        printf("lyircs:[");
                        encodedprintf(lyrics, encoding);
                        printf("]");
                        free(lyrics);
                    }

                }else if(id3v2GetFrameID(currFrame) == SLT || id3v2GetFrameID(currFrame) == SYLT){

                    id3buf language = id3v2GetLanguage(currFrame);
                    id3buf description = id3v2GetDescription(currFrame);
                    int format = id3v2GetTimeStampFormat(currFrame);
                    int contentType = id3v2GetSynchronizedLyricsContentType(currFrame);
                    id3buf text = NULL;
                    int stamp = 0;

                    if(language != NULL){
                        printf("language:[%s] ",language);
                        free(language);
                    }

                    printf("format:[%d] ",format);
                    printf("type:[%d] ",contentType);

                    if(description != NULL){
                        printf("desc:[");
                        encodedprintf(description, encoding);
                        printf("] ");
                        free(description);
                    }

                    printf("lyrics:[");
                    while((text = id3v2GetSynchronizedLyricsValue(currFrame)) != NULL){
                        printf("[");
                        encodedprintf(text,encoding);
                        printf("]");
                        free(text);
                    }
                    printf("] ");

                    id3v2ResetSynchronizedLyricsIter(currFrame);

                    printf("stamp:[");
                    while((stamp = id3v2GetSynchronizedLyricsTimeStamp(currFrame)) != -1){
                        printf("[%d]",stamp);
                    }
                    printf("] ");

                }else if(id3v2GetFrameID(currFrame) == COM || id3v2GetFrameID(currFrame) == COMM){

                    id3buf language = id3v2GetLanguage(currFrame);
                    id3buf description = id3v2GetDescription(currFrame);
                    id3buf value = id3v2GetCommentValue(currFrame);

                    if(language != NULL){
                        printf("language:[%s] ",language);
                        free(language);
                    }

                    if(description != NULL){
                        printf("desc:[");
                        encodedprintf(description,encoding);
                        printf("] ");
                        free(description);
                    }

                    if(value != NULL){
                        printf("value:[");
                        encodedprintf(value, encoding);
                        printf("]");
                        free(value);
                    }

                }else if(id3v2GetFrameID(currFrame) == REV || id3v2GetFrameID(currFrame) == RVA || id3v2GetFrameID(currFrame) == EQU ||
                        id3v2GetFrameID(currFrame) == EQUA || id3v2GetFrameID(currFrame) == RVAD || id3v2GetFrameID(currFrame) == RVRB ||
                        id3v2GetFrameID(currFrame) == RVA2 || id3v2GetFrameID(currFrame) == EQU2){
                    
                    id3buf value = id3v2GetSubjectiveValue(currFrame);

                    if(value != NULL){
                        printf("value:[%p]",value);
                        free(value);
                    }

                }else if(id3v2GetFrameID(currFrame) == PIC || id3v2GetFrameID(currFrame) == APIC){
                    
                    id3buf format = id3v2GetMIMEType(currFrame);
                    int pictureType = id3v2GetPictureType(currFrame);
                    id3buf desc = id3v2GetDescription(currFrame);
                    id3buf picData = id3v2GetPictureValue(currFrame);

                    if(format != NULL){
                        printf("mime:[%s] ",format);
                        free(format);
                        
                    }
                    
                    if(pictureType != -1){
                        printf("type:[%d] ",pictureType);

                    }

                    if(desc != NULL){
                        printf("desc:[");
                        encodedprintf(desc, encoding);
                        free(desc);
                        printf("] ");
                    }

                    if(picData != NULL){
                        printf("picData:[%p] ",picData);
                        free(picData);

                        char fileName[100];
                        sprintf(fileName,"%s%d%s","img",pictureType,".jpg");
                        id3v2SavePicture(fileName,currFrame);
                    }

                }else if(id3v2GetFrameID(currFrame) == GEO || id3v2GetFrameID(currFrame) == GEOB){
                    
                    id3buf mime = id3v2GetMIMEType(currFrame);
                    id3buf fileName = id3v2GetObjectFileName(currFrame);
                    id3buf desc = id3v2GetDescription(currFrame);
                    id3buf obj = id3v2GetGeneralEncapsulatedObjectValue(currFrame);

                    if(mime != NULL){
                        printf("mime:[%s] ",mime);
                    }   

                    if(fileName != NULL){
                        printf("fileName:[");
                        encodedprintf(fileName,encoding);
                        printf("] ");
                    }

                    if(desc != NULL){
                        printf("desc:[");
                        encodedprintf(desc,encoding);
                        printf("] ");
                        free(desc);
                    }

                    if(obj != NULL){
                        printf("object:[%p] ",obj);
                        free(obj);
                        
                        id3v2SaveEncapsulatedObject(currFrame);
                    }

                    if(fileName != NULL){
                        free(fileName);
                    }

                    if(mime != NULL){
                        free(mime);
                    }


                }else if(id3v2GetFrameID(currFrame) == CNT || id3v2GetFrameID(currFrame) == PCNT){
                    printf("plays:[%d] ",id3v2GetPlayCount(currFrame));

                }else if(id3v2GetFrameID(currFrame) == POP || id3v2GetFrameID(currFrame) == POPM){
                    
                    id3buf email = id3v2GetEmail(currFrame);
                    int rating = id3v2GetRating(currFrame);
                    int counter = id3v2GetPlayCount(currFrame);

                    if(email != NULL){
                        printf("email:[%s] ",email);
                        free(email);
                    }

                    if(rating != -1){
                        printf("rating:[%d] ",rating);
                    }

                    if(counter != -1){
                        printf("counter:[%d] ",counter);
                    }

                }else if(id3v2GetFrameID(currFrame) == CRM){
                    id3buf owner = id3v2GetOwnerIdentifier(currFrame);
                    id3buf desc = id3v2GetDescription(currFrame);
                    id3buf encryptedBlock = id3v2GetEncryptedMetaValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(desc);
                    }

                    if(desc != NULL){
                        printf("desc:[%s] ",desc);
                        free(desc);
                    }

                    if(encryptedBlock != NULL){
                        printf("encryptedBlock:[%p] ",encryptedBlock);
                        free(encryptedBlock);
                    }

                }else if(id3v2GetFrameID(currFrame) == AENC){
                    id3buf owner = id3v2GetOwnerIdentifier(currFrame);
                    void *start = id3v2GetPreviewStart(currFrame);
                    int previewlength = id3v2GetPreviewLength(currFrame);
                    id3buf info = id3v2GetAudioEncryptionValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(owner);
                    }

                    if(start != NULL){
                        printf("preview:[%p] ",start);
                    }

                    if(previewlength  != -1){
                        printf("length:[%d] ",previewlength);
                    }

                    if(info != NULL){
                        printf("info:[%p] ",info);
                        free(info);
                    }

                }else if(id3v2GetFrameID(currFrame) == UFI || id3v2GetFrameID(currFrame) == UFID){
                    id3buf owner = id3v2GetOwnerIdentifier(currFrame);
                    id3buf identifier = id3v2GetUniqueFileIdentifierValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(owner);
                    }

                    if(identifier != NULL){
                        printf("identifier:[%s] ",identifier);
                        free(identifier);
                    }

                }else if(id3v2GetFrameID(currFrame) == POSS){
                    int format = id3v2GetTimeStampFormat(currFrame);
                    long pos = id3v2GetPositionSynchronisationValue(currFrame);

                    printf("fomat:[%d] pos:[%ld] ",format,pos);

                }else if(id3v2GetFrameID(currFrame) == USER){
                    id3buf language = id3v2GetLanguage(currFrame);
                    id3buf text = id3v2GetTermsOfUseValue(currFrame);
                    
                    if(language != NULL){
                        printf("language:[%s] ",language);
                        free(language);
                    }

                    if(text != NULL){
                        printf("terms:[");
                        encodedprintf(text, encoding);
                        free(text);
                        printf("] ");
                    }

                }else if(id3v2GetFrameID(currFrame) == OWNE){
                    id3buf price = id3v2GetPrice(currFrame);
                    id3buf date = id3v2GetPunchDate(currFrame);
                    id3buf seller = id3v2GetSeller(currFrame);

                    if(price != NULL){
                        printf("price:[%s] ",price);
                        free(price);
                    }

                    if(date != NULL){
                        printf("date:[%s] ",date);
                        free(date);
                    }

                    if(seller != NULL){
                        printf("seller:[");
                        encodedprintf(seller,encoding);
                        printf("]");
                        free(seller);
                    }


                }else if(id3v2GetFrameID(currFrame) == COMR){
                    id3buf price = id3v2GetPrice(currFrame);
                    id3buf validUntil = id3v2GetValidDate(currFrame);
                    id3buf contract = id3v2GetContractURL(currFrame);
                    int as = id3v2GetCommecialDeliveryMethod(currFrame);
                    id3buf seller = id3v2GetSeller(currFrame);
                    id3buf desc = id3v2GetDescription(currFrame);
                    id3buf mime = id3v2GetMIMEType(currFrame);
                    id3buf logo = id3v2GetCommercialSellerLogo(currFrame);

                    if(price != NULL){
                        printf("price:[%s] ",price);
                        free(price);
                    }   

                    if(validUntil != NULL){
                        printf("validUntil:[%s] ",validUntil);
                        free(validUntil);
                    }
                    if(contract != NULL){
                        printf("contractURL:[%s] ",contract);
                        free(contract);
                    }   
                    if(as != -1){
                        printf("receivedAs:[%d] ",as);
                    }
                    if(seller != NULL){
                        printf("seller:[");
                        encodedprintf(seller, encoding);
                        printf("] ");
                        free(seller);
                    }      
                    if(desc != NULL){
                        printf("desc:[");
                        encodedprintf(desc, encoding);
                        printf("] ");
                        free(desc);
                    }
                    if(mime != NULL){
                        printf("mime:[%s] ",mime);
                    }

                    if(logo != NULL){
                        printf("logo:[%p] ",logo);
                        free(logo);
                    }   

                }else if(id3v2GetFrameID(currFrame) == ENCR){
                    id3buf owner = id3v2GetOwnerIdentifier(currFrame);
                    id3byte methodSymbol = id3v2GetSymbol(currFrame);
                    id3buf encryptionData = id3v2GetEncryptionRegistrationValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(owner);
                    }

                    if(methodSymbol != 0x00){
                        printf("methodSymbol:[%x] ",methodSymbol);
                    }

                    if(encryptionData != NULL){
                        printf("data:[%p] ",encryptionData);
                        free(encryptionData);
                    }
                }else if(id3v2GetFrameID(currFrame) == GRID){
                    id3buf owner = id3v2GetOwnerIdentifier(currFrame);
                    id3byte groupSymbol = id3v2GetSymbol(currFrame);
                    id3buf groupData = id3v2GetGroupIDValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(owner);
                    }

                    if(groupSymbol != 0x00){
                        printf("symbol:[%x] ",groupSymbol);
                    }

                    if(groupData != NULL){
                        printf("data:[%p] ",groupData);
                        free(groupData);
                    }


                }else if(id3v2GetFrameID(currFrame) == PRIV){
                    id3buf owner = id3v2GetOwnerIdentifier(currFrame);
                    id3buf privateData = id3v2GetPrivateValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(owner);
                    }

                    if(privateData != NULL){
                        printf("data:[%p] ",privateData);
                        free(privateData);
                    }
                    
                }else if(id3v2GetFrameID(currFrame) == SIGN){
                    id3byte groupSymbol = id3v2GetSymbol(currFrame);
                    id3buf signature = NULL;

                    if(groupSymbol != 0x00){
                        printf("symbol:[%x] ",groupSymbol);
                    }

                    if(signature != NULL){
                        printf("signature:[%p] ",signature);
                        free(signature);
                    }


                }else if(id3v2GetFrameID(currFrame) == SEEK){
                    int seek = 0;

                    printf("seek:[%d] ", seek);
                }else{
                    printf("parsed and present");
                }

                printf("\n");
            }
        }            
    }
    id3v2ResetIterTag(data->version2);
}


int main(int argc, char *argv[]){

    if(argc != 2){
        exit(EXIT_FAILURE);
    }

    Id3Metadata *data = id3NewMetadataFromFile(argv[1]);
    
    Id3v2FrameId id;
    if(id3v2GetVersion(data->version2) >= 30){
        id = PCNT;
    }else{
        id = CNT;
    }
    
    Id3v2Frame *new = id3v2CreatePlayCounterFrame(id, 100);
    id3v2AddFrameToTag(data->version2, new);
    //printf("[Original]===================================================================\n");
    //metadataPrint(data);

    // Id3List *l = id3v2SearchForFrames(data->version2, id);
    // Id3ListIter *iter = id3NewListIter(l);
    // Id3v2Frame *checkFrame = NULL;
    // while((checkFrame = id3NextListIter(iter)) != NULL){

    //     Id3v2GeneralEncapsulatedObjectBody *body = (Id3v2GeneralEncapsulatedObjectBody *)checkFrame->frame;
    //     Id3v2Frame *testFrame = id3v2CreateGeneralEncapsulatedObjectFrame(id, body->encoding, body->mimeType, body->filename, body->contentDescription, body->encapsulatedObject, body->encapsulatedObjectLen);
    //     id3v2AddFrameToTag(data->version2, testFrame);
    //     //id3v2FreeFrame(testFrame);
    // }
    // id3DestroyList(l);
    // id3FreeListIter(iter);
    
    printf("[edited]=====================================================================\n");
    metadataPrint(data);

    //printf("[Make A Copy]\n");
    //Id3Metadata *data2 = id3CopyMetadata(data);
    //printf("Exact copy| %s\n", id3v1CompareTag(data->version1, data2->version1) == 0 ? "false" : "true");
    //id3v1SetTitle("this is a different title", data2->version1);
    //id3v1SetArtist("I changed the artist", data2->version1);
    //id3v1SetAlbum("I changed the album title", data2->version1);
    //id3v1SetYear(1990, data2->version1);
    //id3v1SetComment("I changed the comment again", data2->version1);
    //id3v1SetGenre(255, data2->version1);
    //id3v1SetTrack(255, data2->version1); 
    ////id3v1ClearTagInformation(data2->version1);
    //metadataPrint(data2);
    //char *json = id3v1ToJSON(data->version1);
    //printf("%s\n",json);
    //free(json);
    //printf("Exact copy| %s\n", id3v1CompareTag(data->version1, data2->version1) == 0 ? "false" : "true");
    //id3v1WriteTag("emptyb.mp3", data2->version1);

    id3FreeMetadata(data);
    //id3FreeMetadata(data2);
        
    return 0;
}