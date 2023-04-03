#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3.h"

void encodedprintf(unsigned char *str, int encoding){
    if(str == NULL){
        return;
    }

    if(encoding == ISO_8859_1 || encoding == UTF8){
        printf("%s",str);
    }
    
    if(encoding == UTF16){
        for(int i = 0; i < strlenUTF16(str); i++){
            printf("%x",str[i]);
        }
    }

    if(encoding == UTF16BE){
        for(int i = 0; i < strlenUTF16BE(str); i++){
            printf("%x",str[i]);
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
            printf("flags. %d%d%d%d\n",id3v2UnsynchronizedIndicator(tag), 
                                    id3v2ExtendedIndicator(tag), 
                                    id3v2ExperimentalIndicator(tag), 
                                    id3v2FooterIndicator(tag));
            printf("tag size. %ld\n",id3v2GetTagSize(tag));

            if(id3v2ExtendedIndicator(tag) == true){
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
                printf("F:%d%d%d%d C:%ld E:%x G:%x|",id3v2FrameAlterPreservationIndicator(currFrame),
                                                    id3v2FrameFileAlterPreservationIndicator(currFrame),
                                                    id3v2FrameReadOnlyIndicator(currFrame),
                                                    id3v2FrameUnsynchronizationIndicator(currFrame),
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
                    
                        unsigned char *desc = id3v2GetDescription(currFrame);
                        unsigned char *value = id3v2GetTextValue(currFrame);
                        
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

                        unsigned char *desc = id3v2GetDescription(currFrame);
                        unsigned char *url = id3v2GetURLValue(currFrame);
                        
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
                    
                    unsigned char *people = id3v2GetInvolvedPeopleListValue(currFrame);
                    
                    printf("people:[");
                    encodedprintf(people, encoding);
                    printf("] ");

                    if(people != NULL){
                        free(people);
                    }  

                }else if(id3v2GetFrameID(currFrame) == MCI || id3v2GetFrameID(currFrame) == MCDI){

                    unsigned char *cdtoc = id3v2GetCDIDValue(currFrame);
                    printf("cdtoc[%s]",cdtoc);

                    if(cdtoc != NULL){
                        free(cdtoc);
                    }

                }else if(id3v2GetFrameID(currFrame) == ETC || id3v2GetFrameID(currFrame) == ETCO){

                    unsigned char type = 0x00;
                    long time = 0;

                    printf("format:[%d] ",id3v2GetTimeStampFormat(currFrame));
                    
                    printf("types:[");
                    while((type = id3v2GetEventTimeCodeType(currFrame)) != 0x00){
                        printf("[%x]",type);
                    }
                    printf("] ");

                    id3v2ResetEventTimeCodeIter(currFrame);

                    printf("stamps:[");
                    while((time = id3v2GetEventTimeCodeTimeStamp(currFrame)) != -1){
                        printf("[%ld]",time);
                    }
                    printf("]");

                }else if(id3v2GetFrameID(currFrame) == ULT || id3v2GetFrameID(currFrame) == USLT){

                    unsigned char *language = id3v2GetLanguage(currFrame);
                    unsigned char *description = id3v2GetDescription(currFrame);
                    unsigned char *lyrics = id3v2GetUnsynchronizedLyrics(currFrame);

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

                    unsigned char *language = id3v2GetLanguage(currFrame);
                    unsigned char *description = id3v2GetDescription(currFrame);
                    int format = id3v2GetTimeStampFormat(currFrame);
                    int contentType = id3v2GetSynchronizedLyricsContentType(currFrame);
                    unsigned char *text = NULL;
                    long stamp = 0;

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
                        printf("[%ld]",stamp);
                    }
                    printf("] ");

                }else if(id3v2GetFrameID(currFrame) == COM || id3v2GetFrameID(currFrame) == COMM){

                    unsigned char *language = id3v2GetLanguage(currFrame);
                    unsigned char *description = id3v2GetDescription(currFrame);
                    unsigned char *value = id3v2GetCommentValue(currFrame);

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
                    
                    unsigned char *value = id3v2GetSubjectiveValue(currFrame);

                    if(value != NULL){
                        printf("value:[%p]",value);
                        free(value);
                    }

                }else if(id3v2GetFrameID(currFrame) == PIC || id3v2GetFrameID(currFrame) == APIC){
                    
                    unsigned char *format = id3v2GetMIMEType(currFrame);
                    int pictureType = id3v2GetPictureType(currFrame);
                    unsigned char *desc = id3v2GetDescription(currFrame);
                    unsigned char *picData = id3v2GetPictureValue(currFrame);

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
                    
                    unsigned char *mime = id3v2GetMIMEType(currFrame);
                    unsigned char *fileName = id3v2GetObjectFileName(currFrame);
                    unsigned char *desc = id3v2GetDescription(currFrame);
                    unsigned char *obj = id3v2GetGeneralEncapsulatedObjectValue(currFrame);

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
                    
                    unsigned char *email = id3v2GetEmail(currFrame);
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
                    unsigned char *owner = id3v2GetOwnerIdentifier(currFrame);
                    unsigned char *desc = id3v2GetDescription(currFrame);
                    unsigned char *encryptedBlock = id3v2GetEncryptedMetaValue(currFrame);

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
                    unsigned char *owner = id3v2GetOwnerIdentifier(currFrame);
                    void *start = id3v2GetPreviewStart(currFrame);
                    int previewlength = id3v2GetPreviewLength(currFrame);
                    unsigned char *info = id3v2GetAudioEncryptionValue(currFrame);

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
                    unsigned char *owner = id3v2GetOwnerIdentifier(currFrame);
                    unsigned char *identifier = id3v2GetUniqueFileIdentifierValue(currFrame);

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
                    unsigned char *language = id3v2GetLanguage(currFrame);
                    unsigned char *text = id3v2GetTermsOfUseValue(currFrame);
                    
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

                }else{
                    printf("parsed and present");
                }

                printf("\n");
            }
        }            
    }

}


int main(int argc, char *argv[]){

    if(argc != 2){
        exit(EXIT_FAILURE);
    }

    Id3Metadata *data = id3NewMetadataFromFile(argv[1]);
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