#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3.h"


int main(int argc, char *argv[]){

    Id3Metadata *data = id3NewMetadataFromFile("01. Paprika.mp3");
    
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

    printf("ID3V2 tag information\n");
    if(hasId3v2(data)){
        
        Id3v2Tag *tag = data->version2;

        if(tag->header == NULL){
            printf("[*]no header\n");
        }else{
            printf("ver. %d.%d\n",tag->header->versionMajor, tag->header->versionMinor);
            printf("flags. %d%d%d%d\n",tag->header->unsynchronisation, 
                                    (tag->header->extendedHeader != NULL) ? true: false, 
                                    tag->header->experimentalIndicator, 
                                    tag->header->footer);
            printf("frame size. %d\n",tag->header->size);

            if(tag->header->extendedHeader != NULL){
                printf("ext size. %d\n",tag->header->extendedHeader->size);
                printf("padding. %d\n",tag->header->extendedHeader->padding);
                printf("crc. %s\n",tag->header->extendedHeader->crc);
                printf("update. %d\n",tag->header->extendedHeader->update);
                printf("tag size restriction %x\n",tag->header->extendedHeader->tagSizeRestriction);
                printf("tag encoding restriction %x\n",tag->header->extendedHeader->encodingRestriction);
                printf("text size restriction %x\n",tag->header->extendedHeader->textSizeRestriction);
                printf("image encoding restriction %x\n",tag->header->extendedHeader->imageEncodingRestriction);
                printf("image size restriction %x\n",tag->header->extendedHeader->imageSizeRestriction);
                
            }else{
                printf("[*]no extended header\n");
            }

        }

        if(tag->frames == NULL){
            printf("[*]no frames\n");

        }else{
            Node *curr = tag->frames->head; 
            
            while(curr != NULL){
                Id3v2Frame *currFrame = (Id3v2Frame *)curr->data;
                printf("%s|",currFrame->header->id);

                if(currFrame->header->id[0] == 'T'){
                    Id3v2TextBody *body = (Id3v2TextBody *)currFrame->frame;

                    if(body->encoding == ISO_8859_1 || body->encoding == UTF8){
                        if(body->description != NULL){
                            printf("desc:[%s] ",body->description);
                        }
                        printf("value:[%s] ",body->value);
                    }

                    if(body->encoding == UTF16){
                        if(body->description != NULL){
                            printf("desc:[");
                            for(int i = 0; i < strlenUTF16(body->description); i++){
                                printf("[%x]",body->description[i]);
                            }
                            printf("] ");
                        }
                        
                        printf("value:[");
                        for(int i = 0; i < strlenUTF16(body->value); i++){
                            printf("[%x]",body->value[i]);
                        }
                        printf("]");
                    }

                    if(body->encoding == UTF16BE){
                        if(body->description != NULL){
                            printf("desc:[");
                            for(int i = 0; i < strlenUTF16BE(body->description); i++){
                                printf("[%x]",body->description[i]);
                            }
                            printf("] ");
                        }
                        printf("value:[");
                        for(int i = 0; i < strlenUTF16BE(body->value); i++){
                            printf("[%x]",body->value[i]);
                        }
                        printf("]");
                    }
                }else if(currFrame->header->idNum == PIC || currFrame->header->idNum == APIC){
                    Id3v2PictureBody *body = (Id3v2PictureBody *)currFrame->frame;

                    printf("formate[%s]",body->format);
                    printf(" picture type[%x]",body->pictureType);
                    if(body->encoding == 0){
                        printf(" desc:[%s]",body->description);
                    }

                    if(body->encoding == 1){
                        printf(" desc:[");
                        for(int i = 0; i < strlenUTF16(body->description); i++){
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

                    if(body->encoding == ISO_8859_1 || body->encoding == UTF8){
                        
                        printf("language:[%s]",body->language);

                        if(body->description != NULL){
                            printf(" desc:[%s]",body->description);
                        }
                        
                        printf(" value:[%s]",body->text);
                    }

                    if(body->encoding == UTF16){

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
                    if(body->encoding == UTF16BE){

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
                curr = curr->next;
            }
        }            


    

    }
    id3FreeMetadata(data);
        
    return 0;
}