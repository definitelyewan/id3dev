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
            printf("ver. %d\n",id3v2ReadVersion(tag));
            printf("flags. %d%d%d%d\n",id3v2ReadUnsynchronizedIndicator(tag), 
                                    id3v2ReadExtendedIndicator(tag), 
                                    id3v2ReadExperimentalIndicator(tag), 
                                    id3v2ReadFooterIndicator(tag));

            if(id3v2ReadExtendedIndicator(tag) == true){
                printf("ext size. %d\n",tag->header->extendedHeader->size);
                printf("padding. %d\n",tag->header->extendedHeader->padding);
                printf("crc. %s\n",(char *)id3v2ReadCrc(tag));
                printf("update. %d\n",tag->header->extendedHeader->update);
                printf("tag size restriction %x\n",id3v2ReadTagSizeRestriction(tag));
                printf("tag encoding restriction %x\n",id3v2ReadEncodingRestriction(tag));
                printf("text size restriction %x\n",id3v2ReadTextSizeRestriction(tag));
                printf("image encoding restriction %x\n",id3v2ReadEncodingRestriction(tag));
                printf("image size restriction %x\n",id3v2ReadTagSizeRestriction(tag));
                
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
                printf("%s|",id3v2ReadFrameStrID(currFrame));
                printf("FS:%ld HS:%ld ID:%d|",id3v2ReadFrameSize(currFrame),
                                            id3v2ReadFrameHeaderSize(currFrame),
                                            id3v2ReadFrameID(currFrame));

                //print flag content
                printf("F:%d%d%d%d%d C:%ld E:%x G:%x|",id3v2ReadFrameAlterPreservationIndicator(currFrame),
                                                    id3v2ReadFrameFileAlterPreservationIndicator(currFrame),
                                                    id3v2ReadFrameReadOnlyIndicator(currFrame),
                                                    id3v2ReadFrameUnsynchronizationIndicator(currFrame),
                                                    id3v2ReadFrameDataLengthIndicator(currFrame),
                                                    id3v2ReadFrameCompressionSize(currFrame),
                                                    id3v2ReadFrameEncryptionMethod(currFrame),
                                                    id3v2ReadFrameGroup(currFrame));
                
                int encoding = id3v2ReadEncoding(currFrame);
                
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
                        break;
                    default:
                        printf("None|");
                }
                
                if(id3v2ReadFrameStrID(currFrame)[0] == 'T'){
                    
                        id3buf desc = id3v2ReadDescription(currFrame);
                        id3buf value = id3v2ReadTextValue(currFrame);
                        
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
                
                }else if(id3v2ReadFrameStrID(currFrame)[0] == 'W' && id3v2ReadFrameID(currFrame) != WCOM){

                        id3buf desc = id3v2ReadDescription(currFrame);
                        id3buf url = id3v2ReadURLValue(currFrame);
                        
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

                }else if(id3v2ReadFrameID(currFrame) == IPL || id3v2ReadFrameID(currFrame) == IPLS){
                    
                    id3buf person = NULL;
                    id3buf job = NULL;

                    while(true){
                        
                        printf("involved:[");
                        person = id3v2ReadInvolvedPeopleListPerson(currFrame);
                        job = id3v2ReadInvolvedPeopleListJob(currFrame);

                        if(person != NULL){
                            encodedprintf(person, id3v2ReadEncoding(currFrame));
                            printf(" ");
                            free(person);
                        }
                        
                        if(job != NULL){
                            encodedprintf(job, id3v2ReadEncoding(currFrame));
                            free(job);
                        }

                        printf("] ");

                        if(!id3v2IterInvolvedPeopleListFrame(currFrame)){
                            break;
                            
                        }
                        
                    }  

                    id3v2ResetInvolvedPeopleListIter(currFrame);

                }else if(id3v2ReadFrameID(currFrame) == MCI || id3v2ReadFrameID(currFrame) == MCDI){

                    id3buf cdtoc = id3v2ReadCDIDValue(currFrame);
                    printf("cdtoc[%s]",cdtoc);

                    if(cdtoc != NULL){
                        free(cdtoc);
                    }

                }else if(id3v2ReadFrameID(currFrame) == ETC || id3v2ReadFrameID(currFrame) == ETCO){

                    while(true){

                        printf("stamp:[type :[%x] time:[%d]]",id3v2ReadEventTimeCodeType(currFrame),id3v2ReadEventTimeCodeTimeStamp(currFrame));    
                        
                        if(!id3v2IterEventTimeCodesFrame(currFrame)){
                            break;
                        }
                    }
                    id3v2ResetEventTimeCodesIter(currFrame);

                }else if(id3v2ReadFrameID(currFrame) == ULT || id3v2ReadFrameID(currFrame) == USLT){

                    id3buf language = id3v2ReadLanguage(currFrame);
                    id3buf description = id3v2ReadDescription(currFrame);
                    id3buf lyrics = id3v2ReadUnsynchronizedLyrics(currFrame);

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

                }else if(id3v2ReadFrameID(currFrame) == SLT || id3v2ReadFrameID(currFrame) == SYLT){

                    id3buf language = id3v2ReadLanguage(currFrame);
                    id3buf description = id3v2ReadDescription(currFrame);
                    int format = id3v2ReadTimeStampFormat(currFrame);
                    int contentType = id3v2ReadSynchronizedLyricsContentType(currFrame);
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
                    while(true){
                        text = id3v2ReadSynchronizedLyricsValue(currFrame);
                        stamp = id3v2ReadSynchronizedLyricsTimeStamp(currFrame);

                        printf("stamp:[%d] ",stamp);

                        printf("text[");
                        encodedprintf(text,encoding);
                        printf("]");
                        free(text);

                        if(!id3v2IterSynchronizedLyricsFrame(currFrame)){
                            break;
                        }
                    }
                    printf("] ");

                    id3v2ResetSynchronizedLyricsIter(currFrame);

                }else if(id3v2ReadFrameID(currFrame) == COM || id3v2ReadFrameID(currFrame) == COMM){

                    id3buf language = id3v2ReadLanguage(currFrame);
                    id3buf description = id3v2ReadDescription(currFrame);
                    id3buf value = id3v2ReadCommentValue(currFrame);

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

                }else if(id3v2ReadFrameID(currFrame) == REV || id3v2ReadFrameID(currFrame) == RVA || id3v2ReadFrameID(currFrame) == EQU ||
                        id3v2ReadFrameID(currFrame) == EQUA || id3v2ReadFrameID(currFrame) == RVAD || id3v2ReadFrameID(currFrame) == RVRB ||
                        id3v2ReadFrameID(currFrame) == RVA2 || id3v2ReadFrameID(currFrame) == EQU2){
                    
                    id3buf value = id3v2ReadSubjectiveValue(currFrame);

                    if(value != NULL){
                        printf("value:[%p]",value);
                        free(value);
                    }

                }else if(id3v2ReadFrameID(currFrame) == PIC || id3v2ReadFrameID(currFrame) == APIC){
                    
                    id3buf format = id3v2ReadMIMEType(currFrame);
                    int pictureType = id3v2ReadPictureType(currFrame);
                    id3buf desc = id3v2ReadDescription(currFrame);
                    id3buf picData = id3v2ReadPictureValue(currFrame);

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
                        id3v2WritePicture(fileName,currFrame);
                    }

                }else if(id3v2ReadFrameID(currFrame) == GEO || id3v2ReadFrameID(currFrame) == GEOB){
                    
                    id3buf mime = id3v2ReadMIMEType(currFrame);
                    id3buf fileName = id3v2ReadObjectFileName(currFrame);
                    id3buf desc = id3v2ReadDescription(currFrame);
                    id3buf obj = id3v2ReadGeneralEncapsulatedObjectValue(currFrame);

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
                        
                        id3v2WriteGeneralEncapsulatedObject(currFrame);
                        free(obj);
                    }

                    if(fileName != NULL){
                        free(fileName);
                    }

                    if(mime != NULL){
                        free(mime);
                    }


                }else if(id3v2ReadFrameID(currFrame) == CNT || id3v2ReadFrameID(currFrame) == PCNT){
                    printf("plays:[%ld] ",id3v2ReadPlayCount(currFrame));

                }else if(id3v2ReadFrameID(currFrame) == POP || id3v2ReadFrameID(currFrame) == POPM){
                    
                    id3buf email = id3v2ReadEmail(currFrame);
                    int rating = id3v2ReadRating(currFrame);
                    unsigned long counter = id3v2ReadPlayCount(currFrame);

                    if(email != NULL){
                        printf("email:[%s] ",email);
                        free(email);
                    }

                    if(rating != -1){
                        printf("rating:[%d] ",rating);
                    }

                    if(counter != -1){
                        printf("counter:[%ld] ",counter);
                    }

                }else if(id3v2ReadFrameID(currFrame) == CRM){
                    id3buf owner = id3v2ReadOwnerIdentifier(currFrame);
                    id3buf desc = id3v2ReadDescription(currFrame);
                    id3buf encryptedBlock = id3v2ReadEncryptedMetaValue(currFrame);

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

                }else if(id3v2ReadFrameID(currFrame) == AENC){
                    id3buf owner = id3v2ReadOwnerIdentifier(currFrame);
                    void *start = id3v2ReadPreviewStart(currFrame);
                    int previewlength = id3v2ReadPreviewLength(currFrame);
                    id3buf info = id3v2ReadAudioEncryptionValue(currFrame);

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

                }else if(id3v2ReadFrameID(currFrame) == UFI || id3v2ReadFrameID(currFrame) == UFID){
                    id3buf owner = id3v2ReadOwnerIdentifier(currFrame);
                    id3buf identifier = id3v2ReadUniqueFileIdentifierValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(owner);
                    }

                    if(identifier != NULL){
                        printf("identifier:[%s] ",identifier);
                        free(identifier);
                    }

                }else if(id3v2ReadFrameID(currFrame) == POSS){
                    int format = id3v2ReadTimeStampFormat(currFrame);
                    unsigned long pos = id3v2ReadPositionSynchronisationValue(currFrame);

                    printf("fomat:[%d] pos:[%ld] ",format,pos);

                }else if(id3v2ReadFrameID(currFrame) == USER){
                    id3buf language = id3v2ReadLanguage(currFrame);
                    id3buf text = id3v2ReadTermsOfUseValue(currFrame);
                    
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

                }else if(id3v2ReadFrameID(currFrame) == OWNE){
                    id3buf price = id3v2ReadPrice(currFrame);
                    id3buf date = id3v2ReadPunchDate(currFrame);
                    id3buf seller = id3v2ReadSeller(currFrame);

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


                }else if(id3v2ReadFrameID(currFrame) == COMR){
                    id3buf price = id3v2ReadPrice(currFrame);
                    id3buf validUntil = id3v2ReadValidDate(currFrame);
                    id3buf contract = id3v2ReadContractURL(currFrame);
                    int as = id3v2ReadCommecialDeliveryMethod(currFrame);
                    id3buf seller = id3v2ReadSeller(currFrame);
                    id3buf desc = id3v2ReadDescription(currFrame);
                    id3buf mime = id3v2ReadMIMEType(currFrame);
                    id3buf logo = id3v2ReadCommercialSellerLogo(currFrame);

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

                }else if(id3v2ReadFrameID(currFrame) == ENCR){
                    id3buf owner = id3v2ReadOwnerIdentifier(currFrame);
                    id3byte methodSymbol = id3v2ReadSymbol(currFrame);
                    id3buf encryptionData = id3v2ReadEncryptionRegistrationValue(currFrame);

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
                }else if(id3v2ReadFrameID(currFrame) == GRID){
                    id3buf owner = id3v2ReadOwnerIdentifier(currFrame);
                    id3byte groupSymbol = id3v2ReadSymbol(currFrame);
                    id3buf groupData = id3v2ReadGroupIDValue(currFrame);

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


                }else if(id3v2ReadFrameID(currFrame) == PRIV){
                    id3buf owner = id3v2ReadOwnerIdentifier(currFrame);
                    id3buf privateData = id3v2ReadPrivateValue(currFrame);

                    if(owner != NULL){
                        printf("owner:[%s] ",owner);
                        free(owner);
                    }

                    if(privateData != NULL){
                        printf("data:[%p] ",privateData);
                        free(privateData);
                    }
                    
                }else if(id3v2ReadFrameID(currFrame) == SIGN){
                    id3byte groupSymbol = id3v2ReadSymbol(currFrame);
                    id3buf signature = NULL;

                    if(groupSymbol != 0x00){
                        printf("symbol:[%x] ",groupSymbol);
                    }

                    if(signature != NULL){
                        printf("signature:[%p] ",signature);
                        free(signature);
                    }


                }else if(id3v2ReadFrameID(currFrame) == SEEK){
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
    
    if(data == NULL){
        perror(argv[1]);
        exit(0);
    }
    
    //make it just utf8
    Id3ListIter *m = id3NewListIter(data->version2->frames);
    Id3v2Frame *mf = NULL;

    while((mf = id3NextListIter(m)) != NULL){
        //printf("%s\n",mf->header->id);
        //id3v2SetEncoding(UTF8, mf);
        //id3v2SetFrameEncryptionMethod(0xD7, mf);
        //id3v2SetFrameReadOnlyIndicator(true, mf);
        //id3v2SetFrameCompressionSize(100, mf);
    }

    id3FreeListIter(m);
    /*size check*/
    FILE *fc = fopen(argv[1], "rb");
    id3byte v[4] = {0,0,0,0};
    fseek(fc, 6, SEEK_SET);
    fread(v, 1, 4, fc);
    printf("[%x][%x][%x][%x]\n",v[0],v[1],v[2],v[3]);
    printf("%d %d\n",syncintDecode(btoi(v,4)), id3v2CalculateTagSize(data->version2));
    fclose(fc);

    unsigned int len = 0;
    id3buf r = id3v2TagToBuffer(&len, data->version2);
    printf("buffer size is %d\n",len);
    if(r == NULL){
        printf("NULL\n");
    }
    
    FILE *fp = fopen("output.mp3","wb");
    fwrite(r,1,len,fp);
    fclose(fp);
    free(r);
    
    /*
    metadataPrint(data);
    
    
    id3buf curr = NULL;
    id3v2SetTitle((id3buf)"😔",strlen("😔"), data->version2);
    id3v2SetArtist((id3buf)"muffin man",strlen("muffin man"), data->version2);
    id3v2SetAlbum((id3buf)"()()()()()brackets?",strlen("()()()()()brackets?"), data->version2);
    id3v2SetAlbumArtist((id3buf)"§§§",strlen("§§§"), data->version2);
    id3v2SetComment((id3buf)"yum",strlen("yum"), data->version2);
    id3v2SetComposer((id3buf)"✝",strlen("✝"), data->version2);
    id3v2SetGenre((id3buf)"trap",strlen("trap"),data->version2);
    id3v2SetTrack((id3buf)"69/420",strlen("69/420"),data->version2);
    id3v2SetDisc((id3buf)"69/420",strlen("69/420"),data->version2);
    id3v2SetYear((id3buf)"9999",strlen("9999"),data->version2);
    id3v2SetLyrics((id3buf)"मेरी छोटी बाइक",strlen("मेरी छोटी बाइक"),data->version2);

    curr = id3v2ReadTitle(UTF8, data->version2);
    printf("Title:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadArtist(UTF8, data->version2);
    printf("Artist:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadAlbumArtist(UTF8, data->version2);
    printf("Album Artist:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadAlbum(UTF8, data->version2);
    printf("Album:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadComposer(UTF8, data->version2);
    printf("Composer:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadYear(UTF8, data->version2);
    printf("Year:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadComment(UTF8, data->version2);
    printf("Comment:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadGenre(UTF8, data->version2);
    printf("Genre:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadTrack(UTF8, data->version2);
    printf("track:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadDisc(UTF8, data->version2);
    printf("disc:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }

    curr = id3v2ReadLyrics(UTF8, data->version2);
    printf("lyrics:[");
    encodedprintf(curr, UTF8);
    printf("]\n");
    if(curr != NULL){
        free(curr);
    }
    
    
    Id3v2FrameId id;
    if(id3v2ReadVersion(data->version2) >= 30){
        id = GEOB;
    }else{
        id = GEO;
    }
    
    Id3v2Frame *f = NULL;
    while((f = id3v2IterTag(data->version2)) != NULL){
        if(id3v2ReadFrameID(f) == id){
            
            //id3v2SetGeneralEncapsulatedObjectValue(NULL,9,f);
            //id3v2SetObjectFileName((id3buf)"tryAgain",strlen("tryAgain"),f);
            //id3v2SetLanguage((id3buf)"CA", f);
            //id3v2SetUnsynchronizedLyrics((id3buf)"nwordswordpword",strlen("nwordswordpword"),f);
            //id3v2SetEncoding(UTF8, f);
            //id3v2RemoveEventTimeCode(60000, f);
            //id3v2SetMIMEType((id3buf)"jpeg", f);
            //id3v2SetCommentValue((id3buf)"wubwub",strlen("wubwub"), f);
            //id3v2RemoveSynchronizedLyric(1200000, f);            


            //id3v2SetDescription(NULL, 0, f);
            //id3v2SetURLValue(NULL, 0, f);
            //id3v2SetDescription((id3buf)"top text... bottom text",id3strlen(
            //(id3buf)"top text... bottom text",ISO_8859_1),f);
            //id3v2SetURLValue((id3buf)"top text... bottom text",id3strlen(
            //(id3buf)"top text... bottom text",ISO_8859_1),f);
            //id3v2SetFrameAlterPreservationIndicator(true, f);
            //id3v2SetFrameFileAlterPreservationIndicator(true, f);
            //id3v2SetFrameReadOnlyIndicator(true, f);
            //id3v2SetFrameUnsynchronizationIndicator(true, f);
            //id3v2SetFrameCompressionSize(100, f);
            //id3v2SetFrameEncryptionMethod(0x99, f);
            //id3v2SetFrameGroup(0x99, f);

        }
        
    }
    id3v2ResetIterTag(data->version2);
    
    printf("[edited]=====================================================================\n");
    metadataPrint(data);
    */
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