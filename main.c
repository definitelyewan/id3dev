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
                
                int encoding = id3v2GetFrameEncoding(currFrame);
                
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
                    
                        unsigned char *desc = id3v2GetFrameDescription(currFrame);
                        unsigned char *value = id3v2GetTextFrameValue(currFrame);
                        
                        printf("desc:[");
                        encodedprintf(desc, id3v2GetFrameEncoding(currFrame));
                        printf("] ");
                        printf("value:[");
                        encodedprintf(value, id3v2GetFrameEncoding(currFrame));
                        printf("]");

                        if(desc != NULL){
                            free(desc);
                        }
                        if(value != NULL){
                            free(value);
                        }
                
                }else if(id3v2GetFrameStrID(currFrame)[0] == 'W' && id3v2GetFrameID(currFrame) != WCOM){

                        unsigned char *desc = id3v2GetFrameDescription(currFrame);
                        unsigned char *url = id3v2GetURLFrameValue(currFrame);
                        
                        printf("desc:[");
                        encodedprintf(desc, id3v2GetFrameEncoding(currFrame));
                        printf("] ");
                        printf("url:[%s]",url);

                        if(desc != NULL){
                            free(desc);
                        }
                        if(url != NULL){
                            free(url);
                        }     

                }else if(id3v2GetFrameID(currFrame) == IPL || id3v2GetFrameID(currFrame) == IPLS){
                    
                    unsigned char *people = id3v2GetInvolvedPeopleListFrameValue(currFrame);
                    
                    printf("people:[");
                    encodedprintf(people, id3v2GetFrameEncoding(currFrame));
                    printf("] ");

                    if(people != NULL){
                        free(people);
                    }  

                }else if(id3v2GetFrameID(currFrame) == MCI || id3v2GetFrameID(currFrame) == MCDI){

                    unsigned char *cdtoc = id3v2GetCDIDFrameValue(currFrame);
                    printf("cdtoc[%s]",cdtoc);

                    if(cdtoc != NULL){
                        free(cdtoc);
                    }

                }else if(id3v2GetFrameID(currFrame) == ETC || id3v2GetFrameID(currFrame) == ETCO){

                    printf("format:[%d] ",id3v2GetFrameTimeStampFormat(currFrame));


                }else if(currFrame->header->idNum == PIC || currFrame->header->idNum == APIC){
                    Id3v2PictureBody *body = (Id3v2PictureBody *)currFrame->frame;

                    printf("formate[%s]",body->format);
                    printf(" picture type[%x]",body->pictureType);
                    if(encoding == ISO_8859_1  || encoding == UTF8){
                        printf(" desc:[%s]",body->description);
                    }

                    if(encoding == UTF16){
                        printf(" desc:[");
                        for(int i = 0; i < strlenUTF16(body->description); i++){
                            printf("[%x]",body->description[i]);
                        }
                        printf("]");
                    }

                    if(encoding == UTF16BE){
                        printf(" desc:[");
                        for(int i = 0; i < strlenUTF16BE(body->description); i++){
                            printf("[%x]",body->description[i]);
                        }
                        printf("]");
                    }

                    char *test = calloc(sizeof(char), 100);
                    
                    sprintf(test,"%s%d%s","img",body->picSize,".jpg");
                    FILE *fp = fopen(test,"wb");
                    
                    fwrite(body->pictureData,1,body->picSize,fp);
                    free(test);
                    fclose(fp);

                }else if(currFrame->header->idNum == COM || currFrame->header->idNum == COMM){
                    Id3v2CommentBody *body = (Id3v2CommentBody *)currFrame->frame;                

                    if(encoding == ISO_8859_1 || encoding == UTF8){
                        
                        printf("language:[%s]",body->language);

                        if(body->description != NULL){
                            printf(" desc:[%s]",body->description);
                        }
                        
                        printf(" value:[%s]",body->text);
                    }

                    if(encoding == UTF16){

                        printf("language:[%s]",body->language);

                        printf(" desc:[");
                        if(body->description != NULL){
                            for(int i = 0; i < strlenUTF16(body->description); i++){
                                printf("[%x]",body->description[i]);
                            }
                        }
                        printf("]");

                        printf(" text:[");
                        if(body->text != NULL){
                            for(int i = 0; i < strlenUTF16(body->text); i++){
                                printf("[%x]",body->text[i]);
                            }
                        }
                        printf("]");
                    }
                    if(encoding == UTF16BE){

                        printf("language:[%s]",body->language);

                        printf(" desc:[");
                        if(body->description != NULL){
                            for(int i = 0; i < strlenUTF16BE(body->description); i++){
                                printf("[%x]",body->description[i]);
                            }
                        }
                        printf("]");

                        printf(" text:[");
                        if(body->text != NULL){
                            for(int i = 0; i < strlenUTF16BE(body->text); i++){
                                printf("[%x]",body->text[i]);
                            }
                        }
                        printf("]");
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