#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2.h"
#include "id3Reader.h"


int main(int argc, char *argv[]){

    Id3v2 *data = Id3v2Tag("17. On GP.mp3");

    if(data == NULL){
        printf("[*]struct overall is null\n");
        exit(0);
    }

    if(data->header == NULL){
        printf("[*]no header\n");
        exit(0);
    }else{
        printf("ver. %d.%d\n",data->header->versionMajor, data->header->versionMinor);
        printf("flags. %d%d%d%d\n",data->header->unsynchronisation, 
                                   (data->header->extendedHeader != NULL) ? true: false, 
                                   data->header->experimentalIndicator, 
                                   data->header->footer);
        printf("frame size. %d\n",data->header->size);

        if(data->header->extendedHeader != NULL){
            printf("ext size. %d\n",data->header->extendedHeader->size);
            printf("padding. %d\n",data->header->extendedHeader->padding);
            printf("crc. %s\n",data->header->extendedHeader->crc);
            printf("update. %d\n",data->header->extendedHeader->update);
            printf("tag size restriction %x\n",data->header->extendedHeader->tagSizeRestriction);
            printf("tag encoding restriction %x\n",data->header->extendedHeader->encodingRestriction);
            printf("text size restriction %x\n",data->header->extendedHeader->textSizeRestriction);
            printf("image encoding restriction %x\n",data->header->extendedHeader->imageEncodingRestriction);
            printf("image size restriction %x\n",data->header->extendedHeader->imageSizeRestriction);
            
        }else{
            printf("[*]no extended header\n");
        }

    }
    
    if(data->frames == NULL){
        printf("[*]no frames\n");
        id3v2FreeTag(data);
        exit(0);
    }
    
    Node *curr = data->frames->head; 
    
    while(curr != NULL){
        
        Id3v2Frame *currFrame = (Id3v2Frame *)curr->data;
        printf("%s|",currFrame->header->id);
        
        if(currFrame->header->id[0] == 'T'){
            Id3v2TextBody *body = (Id3v2TextBody *)currFrame->frame;

            if(body->encoding == 0 || body->encoding == 3){
                if(body->description != NULL){
                    printf("desc:[%s] ",body->description);
                }
                printf("value:[%s] ",body->value);
            }

            if(body->encoding == 1){
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

            if(body->encoding == 2){
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
        }

        if(currFrame->header->id[0] == 'W'){
            Id3v2URLBody *body = (Id3v2URLBody *)currFrame->frame;
            
            if(currFrame->header->idNum == WXX || currFrame->header->idNum == WXXX){
                if(body->encoding == 0){
                    if(body->description != NULL){
                        printf("desc:[%s]",body->description);
                    }
                    
                }

                if(body->encoding == 1){
                    if(body->description != NULL){
                        printf("desc:[");
                        for(int i = 0; i < strlenUTF16(body->description); i++){
                            printf("[%x]",body->description[i]);
                        }
                        printf("] ");
                    }
                }

                printf("value:[%s]",body->url);
            }else{
                printf("value:[%s]",body->url);
            }
        }

        if(currFrame->header->idNum == IPL || currFrame->header->idNum == IPLS){
            Id3v2InvolvedPeopleListBody *body = (Id3v2InvolvedPeopleListBody *)currFrame->frame;
            
            if(body->encoding == 0){
                printf("value:[%s]",body->peopleListStrings);
            }

            if(body->encoding == 1){
                printf("value:[");
                for(int i = 0; i < strlenUTF16(body->peopleListStrings); i++){
                    printf("[%x]",body->peopleListStrings[i]);
                }
                printf("]");
            }
        }

        if(currFrame->header->idNum == MCI || currFrame->header->idNum == MCDI){
            Id3v2MusicCDIdentifierBody *body = (Id3v2MusicCDIdentifierBody *)currFrame->frame;
            printf("value:[%s]",body->cdtoc);
        }

        if(currFrame->header->idNum == ETC || currFrame->header->idNum == ETCO){
            Id3v2EventTimeCodesBody *body = (Id3v2EventTimeCodesBody *)currFrame->frame;

            Node *n = body->eventTimeCodes->head;
            printf("[");
            while(n != NULL){
                Id3v2EventTimesCodeEvent *event = (Id3v2EventTimesCodeEvent *)n->data;
                printf("[stamp:[%ld] event type:[%x]]",event->timeStamp,event->typeOfEvent);
                n = n->next;
            }
            printf("]");
        }

        if(currFrame->header->idNum == STC || currFrame->header->idNum == SYTC){
            Id3v2SyncedTempoCodesBody *body = (Id3v2SyncedTempoCodesBody *)currFrame->frame;
            printf("value:[%s]",body->tempoData);
        }

        if(currFrame->header->idNum == ULT || currFrame->header->idNum == USLT){
            Id3v2UnsynchronisedLyricsBody *body = (Id3v2UnsynchronisedLyricsBody *)currFrame->frame; 
            printf("language:[%s]",body->language);

            if(body->encoding == 0){
                printf(" desc:[%s]",body->descriptor);
                printf(" lyrics:[%s]",body->lyrics);
            }
            
            if(body->encoding == 1){
                printf("desc:[");
                for(int i = 0; i < strlenUTF16(body->descriptor); i++){
                    printf("[%x]",body->descriptor[i]);
                }
                printf("]");

                printf("lyrics:[");
                for(int i = 0; i < strlenUTF16(body->lyrics); i++){
                    printf("[%x]",body->lyrics[i]);
                }
                printf("]");            
            }        
        }

        if(currFrame->header->idNum == SLT || currFrame->header->idNum == SYLT){
            Id3v2SynchronisedLyricsBody *body = (Id3v2SynchronisedLyricsBody *)currFrame->frame;
        
            printf("language:[%s]",body->language);

            if(body->encoding == 0){
                printf(" desc:[%s]",body->descriptor);
                if(body->lyrics != NULL){
                    Node *n = body->lyrics->head;
                    
                    printf(" lyrics:[");
                    while(n != NULL){
                        Id3v2StampedLyric *l = (Id3v2StampedLyric *)n->data;
                        printf("[text:[%s] stamp:[%ld]]",l->text,l->timeStamp);
                        n = n->next;
                    }
                    printf("]");
                }

            }

            if(body->encoding == 1){
                
                printf("desc:[");
                for(int i = 0; i < strlenUTF16(body->descriptor); i++){
                    printf("[%x]",body->descriptor[i]);
                }
                
                if(body->lyrics != NULL){
                    Node *n = body->lyrics->head;
                    
                    printf(" lyrics:[");
                    while(n != NULL){
                        Id3v2StampedLyric *l = (Id3v2StampedLyric *)n->data;
                        
                        printf("text:[");
                        for(int i = 0; i < strlenUTF16(l->text); i++){
                            printf("[%x]",l->text[i]);
                        }
                        printf("]"); 
                        printf(" stamp:[%ld]]",l->timeStamp);
                        n = n->next;
                    }
                    printf("]");
                }
            }
        }

        if(currFrame->header->idNum == COM || currFrame->header->idNum == COMM){
            Id3v2CommentBody *body = (Id3v2CommentBody *)currFrame->frame;                

            if(body->encoding == 0){
                
                printf("language:[%s]",body->language);

                if(body->description != NULL){
                    printf(" desc:[%s]",body->description);
                }
                
                printf(" value:[%s]",body->text);
            }

            if(body->encoding == 1){

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
        }

        if(currFrame->header->idNum == PIC || currFrame->header->idNum == APIC){
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
            
        
        }

        if(currFrame->header->idNum == GEO || currFrame->header->idNum == GEOB){
            Id3v2GeneralEncapsulatedObjectBody *body = (Id3v2GeneralEncapsulatedObjectBody *)currFrame->frame;
            
            printf("mine type:[%s]",body->mimeType);

            if(body->encoding == 0){
                if(body->filename != NULL){
                    printf(" file name:[%s]",body->filename);
                }
                
                if(body->contentDescription != NULL){
                    printf(" content desc:[%s]",body->contentDescription);
                }                
            }

            if(body->encoding == 1){
                if(body->filename != NULL){
                    printf(" file name:[");
                    for(int i = 0; i < strlenUTF16(body->filename); i++){
                        printf("[%x]",body->filename[i]);
                    }
                    printf("]");
                }

                if(body->contentDescription != NULL){
                    printf(" content desc:[");
                    for(int i = 0; i < strlenUTF16(body->contentDescription); i++){
                        printf("[%x]",body->contentDescription[i]);
                    }
                    printf("]");    
                }

            }

            if(body->encapsulatedObject != NULL){
                FILE *fp = fopen("test.png","wb");
                fwrite(body->encapsulatedObject,1,body->encapsulatedObjectLen,fp);
                fclose(fp);
            }
        }

        if(currFrame->header->idNum == CNT || currFrame->header->idNum == PCNT){
            Id3v2PlayCounterBody *body = (Id3v2PlayCounterBody *)currFrame->frame;

            if(body->counter != NULL){
                printf("counter");
            }
        }

        if(currFrame->header->idNum == POP || currFrame->header->idNum == POPM){
            Id3v2PopularBody *body = (Id3v2PopularBody *)currFrame->frame;

            if(body->email != NULL){
                printf("email:[%s]",body->email);
            }

            printf(" rating:[%d]",body->rating);

            if(body->counter != NULL){
                printf(" counter");
            }
        }

        if(currFrame->header->idNum == CRM){
            Id3v2EncryptedMetaBody *body = (Id3v2EncryptedMetaBody *)currFrame->frame;
            
            printf("block len:[%d] ",body->encryptedDatablockLen);

            if(body->ownerIdentifier != NULL){
                printf("owner id:[%s] ",body->ownerIdentifier);
            }

            if(body->content != NULL){
                printf("content:[%s] ",body->content);
            }
            
        }

        if(currFrame->header->idNum == CRA || currFrame->header->idNum == AENC){
            Id3v2AudioEncryptionBody *body = (Id3v2AudioEncryptionBody *)currFrame->frame;
            
            printf("preview len:[%d] ",body->previewLength);
            printf("encryption len:[%d] ",body->encryptionInfoLen);

            if(body->ownerIdentifier != NULL){
                printf("owner id:[%s] ",body->ownerIdentifier);
            }


        }

        if(currFrame->header->idNum == UFI || currFrame->header->idNum == UFID){
            Id3v2UniqueFileIdentifierBody *body = (Id3v2UniqueFileIdentifierBody *)currFrame->frame;
            if(body->ownerIdentifier != NULL){
                printf("owner id:[%s] ",body->ownerIdentifier);
            }

            if(body->identifier != NULL){
                printf("id:[%s] ",body->identifier);
            }
        }

        if(currFrame->header->idNum == POSS){
            Id3v2PositionSynchronisationBody *body = (Id3v2PositionSynchronisationBody *)currFrame->frame;

            printf("pos:[%ld] ",body->pos);

        }

        if(currFrame->header->idNum == USER){
            Id3v2TermsOfUseBody *body = (Id3v2TermsOfUseBody *)currFrame->frame;

            printf("language:[%s] ",body->language);

            if(body->text != NULL){
                printf("text:[%s] ",body->text);
            }
            
        }

        if(currFrame->header->idNum == OWNE){
            Id3v2OwnershipBody *body = (Id3v2OwnershipBody *)currFrame->frame;

            if(body->pricePayed != NULL){
                printf("price:[%s] ",body->pricePayed);
            }

            if(body->dateOfPunch != NULL){
                printf("date:[%s] ",body->dateOfPunch);
            }

            if(body->seller != NULL){
                
                if(body->encoding == 0){
                    printf("seller:[%s] ",body->seller);
                }

                if(body->encoding == 1){
                    printf("seller[");
                    for(int i = 0; i < strlenUTF16(body->seller); i++){
                        printf("[%x]",body->seller[i]);
                    }
                    printf("] ");
                }
                
            }
        }

        if(currFrame->header->idNum == COMR){
            Id3v2CommercialBody *body = (Id3v2CommercialBody *)currFrame->frame;
        
            if(body->priceString != NULL){
                printf("price:[%s] ",body->priceString);
            }

            if(body->validUntil != NULL){
                printf("valid:[%s] ",body->validUntil);
            }

            if(body->contractURL != NULL){
                printf("url:[%s] ",body->contractURL);
            }

            printf("receivedAs:[%x] ",body->receivedAs);

            if(body->nameOfSeller != NULL){
                printf("name of seller:[%s] ",body->nameOfSeller);
            }

            if(body->description != NULL){
                printf("description:[%s] ",body->description);
            }

            if(body->mimeType != NULL){
                printf("mime type:[%s] ",body->mimeType);
            }
        
        }

        printf("\n");
        curr = curr->next;
    }



    id3v2FreeTag(data);
    
    return 0;
}