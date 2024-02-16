#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "LinkedList.h"
#include "id3v2Context.h"

//djb2 algorithm for stings
unsigned long id3v2djb2(char *str){
    
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/**
 * @brief Creates a context
 * 
 * @param type 
 * @param key 
 * @param max 
 * @param min 
 * @return Id3v2ContentContext* 
 */
Id3v2ContentContext *id3v2CreateContentContext(Id3v2ContextType type, size_t key, size_t max, size_t min){

    Id3v2ContentContext *context = malloc(sizeof(Id3v2ContentContext));

    context->type = type;
    context->key = key;
    context->max = max;
    context->min = min;

    return context;

}

/**
 * @brief Destroys a content context
 * 
 * @param toDelete 
 */
void id3v2DestroyContentContext(Id3v2ContentContext **toDelete){
    
    if(*toDelete){
        free(*toDelete);
        *toDelete = NULL;
        toDelete = NULL;
    }
}

/**
 * 
 * List/Hash API required functions
 *  
 */

/**
 * @brief Frees a context when it is in a List
 * 
 * @param toBeDeleted 
 */
void id3v2DeleteContentContext(void *toBeDeleted){
    id3v2DestroyContentContext((Id3v2ContentContext **) &toBeDeleted);
}

/**
 * @brief Compares two contexts and returns the difference
 * 
 * @param first 
 * @param second 
 * @return int 
 */
int id3v2CompareContentContext(const void *first, const void *second){
    
    Id3v2ContentContext *c1 = NULL;
    Id3v2ContentContext *c2 = NULL;
    int diff = 0;

    c1 = (Id3v2ContentContext *) first;
    c2 = (Id3v2ContentContext *) second;

    diff = c1->type - c2->type;
    if(diff != 0)
        return diff;

    diff = c1->key - c2->key;
    if(diff != 0)
        return diff;

    diff = (int)c1->min - (int)c2->min;
    if(diff != 0)
        return diff;

    diff = (int)c1->max - (int)c2->max;
    return diff;
}

/**
 * @brief Generates a sting that represents a context
 * 
 * @param toBePrinted 
 * @return char* 
 */
char *id3v2PrintContentContext(const void *toBePrinted){
    
    Id3v2ContentContext *c = (Id3v2ContentContext *) toBePrinted;

    // 40 chars for the below string
    char *str = malloc(sizeof(long) + sizeof(long) + sizeof(int) + sizeof(Id3v2ContextType) + 40);

    sprintf(str,"Type: %d, Key: %ld, min: %ld, max: %ld\n", c->type, c->key, c->min, c->max);

    return str;
}

/**
 * @brief Performs a deep copy of a context
 * 
 * @param toBeCopied 
 * @return void* 
 */
void *id3v2CopyContentContext(const void *toBeCopied){

    Id3v2ContentContext *copy = (Id3v2ContentContext *) toBeCopied;
    Id3v2ContentContext *ret = malloc(sizeof(Id3v2ContentContext));

    ret->key = copy->key;
    ret->max = copy->max;
    ret->min = copy->min;
    ret->type = copy->type;
    
    return (void *)ret;
}

/**
 * 
 * Context list generation
 * 
 */

/**
 * @brief Generates the required contexts for a text frame
 * 
 * @return List* 
 */
List *id3v2CreateTextFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required contexts for a user defined text frame 
 * 
 * @return List* 
 */
List *id3v2CreateUserDefinedTextFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // desc
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required contexts for a URL frame
 * 
 * @return List* 
 */
List *id3v2CreateURLFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // url
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required contexts for a user defined URL frame
 * 
 * @return List* 
 */
List *id3v2CreateUserDefinedURLFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // url
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;

}

/**
 * @brief Generates the required contexts for an attached picture frame depending on the version on ID3v2
 * @details will be underfined until i can find a better way or more contexts
 * @param version 
 * @return List* 
 */
List *id3v2CreateAttachedPictureFrameContext(unsigned int version){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // image format
    switch(version){
        case ID3V2_TAG_VERSION_2:
            // format is $xx xx xx
            toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("format"), 3, 1);
            break;

        case ID3V2_TAG_VERSION_3:
        case ID3V2_TAG_VERSION_4:
            // format is latin1 
            toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("format"), UINT_MAX, 1);
            break;

        default:
            // something it wrong
            toAdd = (void *) id3v2CreateContentContext(unknown_context, id3v2djb2("format"), UINT_MAX, 1);
            break;
    }

    listInsertBack(l, toAdd);

    // picture type
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("type"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

List *id3v2CreateAudioSeekPointIndexFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief generates the required context for an audio encryption frame
 * 
 * @return List* 
 */
List *id3v2CreateAudioEncryptionFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // identifier
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // preview start
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("start"), 2, 2);
    listInsertBack(l, toAdd);

    // length
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("length"), 2, 2);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required contexts for a comment frame
 * 
 * @return List* 
 */
List *id3v2CreateCommentFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a commercial frame
 * 
 * @return List* 
 */
List *id3v2CreateCommercialFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // price
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("price"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // date
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("date"), 8, 1);
    listInsertBack(l, toAdd);

    // url
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // recived as (type)
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("type"), 1, 1);
    listInsertBack(l, toAdd);

    // name
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // format
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("format"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for an encrypted meta frame
 * 
 * @return List* 
 */
List *id3v2CreateEncryptedMetaFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // identifier
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // content
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("content"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for an encryption method (ENCR) or group identification (GRID) frame
 * 
 * @return List* 
 */
List *id3v2CreateRegistrationFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // identifier
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // symbol
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;

}

/**
 * @brief Generates the required context for a music CD identifier frame
 * 
 * @return List* 
 */
List *id3v2CreateMusicCDIdentifierFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), 804, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a play counter frame
 * 
 * @return List* 
 */
List *id3v2CreatePlayCounterFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);


    return l;
}

/**
 * @brief Generates the required context for a equlization frame given a version
 * 
 * @param version 
 * @return List* 
 */
List *id3v2CreateEqulizationFrameContext(unsigned int version){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);
    void *toAdd = NULL;

    switch(version){
        // EQU and EQUA
        case ID3V2_TAG_VERSION_2:
        case ID3V2_TAG_VERSION_3:
            
            // adjustment
            toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("adjustment"), 1, 1);
            listInsertBack(l, toAdd);

            // increment decrement
            toAdd = (void *) id3v2CreateContentContext(bit_context, id3v2djb2("unary"), 1, 1);
            listInsertBack(l, toAdd);

            // frequency
            toAdd = (void *) id3v2CreateContentContext(bit_context, id3v2djb2("frequency"), 15, 15);
            listInsertBack(l, toAdd);

            // volume (adjustment dependant)
            toAdd = (void *) id3v2CreateContentContext(adjustment_context, id3v2djb2("volume"), UINT_MAX, 1);
            listInsertBack(l, toAdd);

            // iter through the last 3 limt is int max but a frames data will 100% run out before this
            toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 1);
            listInsertBack(l, toAdd);
            break;

        // EQU2  
        case ID3V2_TAG_VERSION_4:
            
            // symbol
            toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
            listInsertBack(l, toAdd);

            // identifier
            toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
            listInsertBack(l, toAdd);

            // volume
            toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("volume"), 2, 2);
            listInsertBack(l, toAdd);

            // iter through the last 2 limt is int max but a frames data will 100% run out before this
            toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 2);
            listInsertBack(l, toAdd);
            break;

        default:
            toAdd = (void *) id3v2CreateContentContext(unknown_context, id3v2djb2("unkown"), 1, 1);
            listInsertBack(l, toAdd);
            break;
    }

    return l;
}

/**
 * @brief Generates the required context for an event timing codes frame
 * 
 * @return List* 
 */
List *id3v2CreateEventTimingCodesFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // select format
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // type
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("type"), 1, 1);
    listInsertBack(l, toAdd);

    // stamp
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("stamp"), 4, 4);
    listInsertBack(l, toAdd);

    // iter from the type onward
    toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a general encapsulated object frame
 * 
 * @return List* 
 */
List *id3v2CreateGeneralEncapsulatedObjectFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // format
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("format"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // file name
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for Involved peoples list frame in ID3V2.2 and ID3V2.3
 * 
 * @return List* 
 */
List *id3v2CreateInvolvedPeopleListFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // name
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // iter from name onward
    toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a linked information frame
 * 
 * @return List* 
 */
List *id3v2CreateLinkedInformationFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // url
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a MPEG lookup table frame
 * @details This frame does not have full support but its use is probably so rare that nobody will care
 * @return List* 
 */
List *id3v2CreateMPEGLocationLookupTableFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief generates the required context for an ownersip frame
 * 
 * @return List* 
 */
List *id3v2CreateOwnershipFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // price
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("price"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // date
    toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("date"), 8, 8);
    listInsertBack(l, toAdd);

    // date
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("name"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;

}

/**
 * @brief Generate the required context for a popularimeter frame
 * 
 * @return List* 
 */
List *id3v2CreatePopularimeterFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // email
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // symbol / rating
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // counter
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a position synchronisation frame
 * 
 * @return List* 
 */
List *id3v2CreatePositionSynchronisationFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // format
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("format"), 1, 1);
    listInsertBack(l, toAdd);

    // stamp
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("stamp"), 4, 4);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a private frame
 * 
 * @return List* 
 */
List *id3v2CreatePrivateFrameContext(void){
    
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // email
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("identifier"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);
    
    return l;
}

/**
 * @brief Generates the required context for a recommended buffer size frame
 * 
 * @return List* 
 */
List *id3v2CreateRecommendedBufferSizeFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // buffer size
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("buffer"), 3, 3);
    listInsertBack(l, toAdd);

    // bit
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("flag"), 1, 1);
    listInsertBack(l, toAdd);

    // offset
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("offset"), 4, 0);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a volume adjustment frame
 * @details at the time being i do not have a good way of implementing this so the entire frame is one block.
 * maybe post processing functions or more contexts?
 * @param version 
 * @return List* 
 */
List *id3v2CreateRelativeVolumeAdjustmentFrameContext(unsigned int version){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // data
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required contexts for a reverb frame
 * 
 * @return List* 
 */
List *id3v2CreateReverbFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // left
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("left"), 2, 2);
    listInsertBack(l, toAdd);

    // right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("right"), 2, 2);
    listInsertBack(l, toAdd);

    // bounce left
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("bounce left"), 1, 1);
    listInsertBack(l, toAdd);

    // bounce right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("bounce right"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback left 2 left
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback ll"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback left 2 right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback lr"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback right 2 right
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback rr"), 1, 1);
    listInsertBack(l, toAdd);

    // feedback right 2 left
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("feedback rl"), 1, 1);
    listInsertBack(l, toAdd);

    // premix l
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("p left"), 1, 1);
    listInsertBack(l, toAdd);

    // premix r
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("p right"), 1, 1);
    listInsertBack(l, toAdd);
    return l;


}

/**
 * @brief Generates the required context for a seek point index frame
 * 
 * @return List* 
 */
List *id3v2CreateSeekFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // offset
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("offset"), 4, 4);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a signature frame
 * 
 * @return List* 
 */
List *id3v2CreateSignatureFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // symbol
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required context for a synchronised lyric frame
 * 
 * @return List* 
 */
List *id3v2CreateSynchronisedLyricFrameContext(void){
    
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 3);
    listInsertBack(l, toAdd);

    // format
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("format"), 1, 1);
    listInsertBack(l, toAdd);

    // symbol
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("symbol"), 1, 1);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // stamp
    toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("stamp"), 4, 4);
    listInsertBack(l, toAdd);

    // iter
    toAdd = (void *) id3v2CreateContentContext(iter_context, id3v2djb2("iter"), UINT_MAX, 5);
    listInsertBack(l, toAdd);
    
    return l;
}

/**
 * @brief Generates the required context for a synced tempo codes frame
 * @details this frame is not fully supported as i do not have a good way of dealing
 * with varibale tempo data as of right now. maybe postprocessing or more contexts?
 * @return List* 
 */
List *id3v2CreateSynchronisedTempoCodesFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // format
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("format"), 1, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates the required contexts for a unique file identifer frame
 * 
 * @return List* 
 */
List *id3v2CreateUniqueFileIdentifierFrameContext(void){
    
    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // url
    void *toAdd = (void *) id3v2CreateContentContext(latin1Encoding_context, id3v2djb2("url"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // data
    toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("data"), 64, 1);
    listInsertBack(l, toAdd);

    return l;
}

List *id3v2CreateTermsOfUseFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}


/**
 * @brief Generates the required contexts for a unsynced lyrics frame
 * 
 * @return List* 
 */
List *id3v2CreateUnsynchronisedLyricFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(numeric_context, id3v2djb2("encoding"), 1, 1);
    listInsertBack(l, toAdd);

    // language
    toAdd = (void *) id3v2CreateContentContext(noEncoding_context, id3v2djb2("language"), 3, 3);
    listInsertBack(l, toAdd);

    // desc
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("desc"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    // text
    toAdd = (void *) id3v2CreateContentContext(encodedString_context, id3v2djb2("text"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;

}

/**
 * @brief generates the required context for a generic frame with unknown structure
 * 
 * @return List* 
 */
List *id3v2CreateGenericFrameContext(void){

    List *l = listCreate(id3v2PrintContentContext, id3v2DeleteContentContext, id3v2CompareContentContext, id3v2CopyContentContext);

    // encoding
    void *toAdd = (void *) id3v2CreateContentContext(binary_context, id3v2djb2("?"), UINT_MAX, 1);
    listInsertBack(l, toAdd);

    return l;
}

/**
 * @brief Generates a default map of pairings between frame IDs and the context needed to parse them.
 * This works for all major versions of ID3v2.
 * 
 * @param version 
 * @return HashTable* 
 */
HashTable *id3v2CreateDefaultIdentiferContextPairings(unsigned int version){

    size_t minFrameContexts = 64;
    HashTable *table = hashTableCreate(minFrameContexts, id3v2DeleteContentContext, id3v2PrintContentContext, id3v2CopyContentContext);


    switch(version){
        
        case ID3V2_TAG_VERSION_2:
        
            hashTableInsert(table, "BUF", id3v2CreateRecommendedBufferSizeFrameContext());

            hashTableInsert(table, "CNT", id3v2CreatePlayCounterFrameContext());
            
            hashTableInsert(table, "COM", id3v2CreateCommentFrameContext());
            hashTableInsert(table, "CRA", id3v2CreateAudioEncryptionFrameContext());
            hashTableInsert(table, "CRM", id3v2CreateEncryptedMetaFrameContext());

            hashTableInsert(table, "ETC", id3v2CreateEventTimingCodesFrameContext());
            hashTableInsert(table, "EQU", id3v2CreateEqulizationFrameContext(version));

            hashTableInsert(table, "GEO", id3v2CreateGeneralEncapsulatedObjectFrameContext());

            hashTableInsert(table, "IPL", id3v2CreateInvolvedPeopleListFrameContext());

            hashTableInsert(table, "LNK", id3v2CreateLinkedInformationFrameContext());

            hashTableInsert(table, "MCI", id3v2CreateMusicCDIdentifierFrameContext());
            hashTableInsert(table, "MLL", id3v2CreateMPEGLocationLookupTableFrameContext());

            hashTableInsert(table, "PIC", id3v2CreateAttachedPictureFrameContext(version));
            hashTableInsert(table, "POP", id3v2CreatePopularimeterFrameContext());

            hashTableInsert(table, "REV", id3v2CreateReverbFrameContext());
            hashTableInsert(table, "RVA", id3v2CreateRelativeVolumeAdjustmentFrameContext(version));

            hashTableInsert(table, "SLT", id3v2CreateSynchronisedLyricFrameContext());
            hashTableInsert(table, "STC", id3v2CreateSynchronisedTempoCodesFrameContext());

            hashTableInsert(table, "TAL", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TBP", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TCM", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TCO", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TCR", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDA", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TDY", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TEN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TFT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIM", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TKE", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TLA", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TLE", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TMT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOA", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOF", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOL", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOR", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TP1", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TP2", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TP3", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TP4", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPA", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPB", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TRC", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TRD", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TRK", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TSI", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TSS", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TT1", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TT2", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TT3", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TXT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TXX", id3v2CreateUserDefinedTextFrameContext());
            hashTableInsert(table, "TYE", id3v2CreateTextFrameContext()); 

            hashTableInsert(table, "UFI", id3v2CreateUniqueFileIdentifierFrameContext());
            hashTableInsert(table, "ULT", id3v2CreateUnsynchronisedLyricFrameContext());

            hashTableInsert(table, "WAF", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WAR", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WAS", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WCM", id3v2CreateURLFrameContext()); 
            hashTableInsert(table, "WCP", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WPB", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WXX", id3v2CreateUserDefinedURLFrameContext());

            break;
        case ID3V2_TAG_VERSION_3:

            hashTableInsert(table, "AENC", id3v2CreateAudioEncryptionFrameContext());
            hashTableInsert(table, "APIC", id3v2CreateAttachedPictureFrameContext(version));

            hashTableInsert(table, "COMM", id3v2CreateCommentFrameContext());
            hashTableInsert(table, "COMR", id3v2CreateCommercialFrameContext());

            hashTableInsert(table, "ENCR", id3v2CreateRegistrationFrameContext());
            hashTableInsert(table, "EQUA", id3v2CreateEqulizationFrameContext(version));
            hashTableInsert(table, "ETCO", id3v2CreateEventTimingCodesFrameContext());

            hashTableInsert(table, "GEOB", id3v2CreateGeneralEncapsulatedObjectFrameContext());
            hashTableInsert(table, "GRID", id3v2CreateRegistrationFrameContext());

            hashTableInsert(table, "IPLS", id3v2CreateInvolvedPeopleListFrameContext());

            hashTableInsert(table, "LINK", id3v2CreateLinkedInformationFrameContext());
            
            hashTableInsert(table, "MCDI", id3v2CreateMusicCDIdentifierFrameContext());
            hashTableInsert(table, "MLLT", id3v2CreateMPEGLocationLookupTableFrameContext());
            
            hashTableInsert(table, "OWNE", id3v2CreateOwnershipFrameContext());
            
            hashTableInsert(table, "PRIV", id3v2CreatePrivateFrameContext());
            hashTableInsert(table, "PCNT", id3v2CreatePlayCounterFrameContext());
            hashTableInsert(table, "POPM", id3v2CreatePopularimeterFrameContext());
            hashTableInsert(table, "POSS", id3v2CreatePositionSynchronisationFrameContext());

            hashTableInsert(table, "RBUF", id3v2CreateRecommendedBufferSizeFrameContext());
            hashTableInsert(table, "RVAD", id3v2CreateRelativeVolumeAdjustmentFrameContext(version));
            hashTableInsert(table, "RVRB", id3v2CreateReverbFrameContext());

            hashTableInsert(table, "SYLT", id3v2CreateSynchronisedLyricFrameContext());
            hashTableInsert(table, "SYTC", id3v2CreateSynchronisedTempoCodesFrameContext());

            hashTableInsert(table, "TALB", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TBPM", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TCOM", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TCON", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TCOP", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDAT", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TDLY", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TENC", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TEXT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TFLT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIME", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TIT1", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIT2", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIT3", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TKEY", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TLAN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TLEN", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TMED", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TOAL", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TOFN", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TOLY", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TOPE", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TORY", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TOWN", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TPE1", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TPE2", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TPE3", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TPE4", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TPOS", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TPUB", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TRCK", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TRDA", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TRSN", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TRSO", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TSIZ", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TSRC", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TSSE", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TYER", id3v2CreateTextFrameContext()); 
            hashTableInsert(table, "TXXX", id3v2CreateUserDefinedTextFrameContext());

            hashTableInsert(table, "UFID", id3v2CreateUniqueFileIdentifierFrameContext());
            hashTableInsert(table, "USER", id3v2CreateTermsOfUseFrameContext());
            hashTableInsert(table, "USLT", id3v2CreateUnsynchronisedLyricFrameContext());

            hashTableInsert(table, "WCOM", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WCOP", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WOAF", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WOAR", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WOAS", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WORS", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WPAY", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WPUB", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WXXX", id3v2CreateUserDefinedURLFrameContext());

            break;
        case ID3V2_TAG_VERSION_4:
            hashTableInsert(table, "AENC", id3v2CreateAudioEncryptionFrameContext());
            hashTableInsert(table, "APIC", id3v2CreateAttachedPictureFrameContext(version));
            hashTableInsert(table, "ASPI", id3v2CreateAudioSeekPointIndexFrameContext());

            hashTableInsert(table, "COMM", id3v2CreateCommentFrameContext());
            hashTableInsert(table, "COMR", id3v2CreateCommercialFrameContext());

            hashTableInsert(table, "ENCR", id3v2CreateRegistrationFrameContext());
            hashTableInsert(table, "EQU2", id3v2CreateEqulizationFrameContext(version));
            hashTableInsert(table, "ETCO", id3v2CreateEventTimingCodesFrameContext());

            hashTableInsert(table, "GEOB", id3v2CreateGeneralEncapsulatedObjectFrameContext());
            hashTableInsert(table, "GRID", id3v2CreateRegistrationFrameContext());

            hashTableInsert(table, "LINK", id3v2CreateLinkedInformationFrameContext());

            hashTableInsert(table, "MCDI", id3v2CreateMusicCDIdentifierFrameContext());
            hashTableInsert(table, "MLLT", id3v2CreateMPEGLocationLookupTableFrameContext());

            hashTableInsert(table, "OWNE", id3v2CreateOwnershipFrameContext());

            hashTableInsert(table, "PRIV", id3v2CreatePrivateFrameContext());
            hashTableInsert(table, "PCNT", id3v2CreatePlayCounterFrameContext());
            hashTableInsert(table, "POPM", id3v2CreatePopularimeterFrameContext());
            hashTableInsert(table, "POSS", id3v2CreatePositionSynchronisationFrameContext());

            hashTableInsert(table, "RBUF", id3v2CreateRecommendedBufferSizeFrameContext());
            hashTableInsert(table, "RVA2", id3v2CreateRelativeVolumeAdjustmentFrameContext(version));
            hashTableInsert(table, "RVRB", id3v2CreateReverbFrameContext());

            hashTableInsert(table, "SEEK", id3v2CreateSeekFrameContext());
            hashTableInsert(table, "SIGN", id3v2CreateSignatureFrameContext());
            hashTableInsert(table, "SYLT", id3v2CreateSynchronisedLyricFrameContext());
            hashTableInsert(table, "SYTC", id3v2CreateSynchronisedTempoCodesFrameContext());

            hashTableInsert(table, "TALB", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TBPM", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TCOM", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TCON", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TCOP", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDEN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDLY", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDOR", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDRC", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDRL", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TDTG", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TENC", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TEXT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TFLT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIPL", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIT1", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIT2", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TIT3", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TKEY", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TLAN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TLEN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TMCL", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TMED", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TMOO", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOAL", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOFN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOLY", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOPE", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TOWN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPE1", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPE2", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPE3", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPE4", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPOS", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPRO", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TPUB", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TRCK", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TRSN", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TRSO", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TSOA", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TSOP", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TSOT", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TSRC", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TSSE", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TSST", id3v2CreateTextFrameContext());
            hashTableInsert(table, "TXXX", id3v2CreateUserDefinedTextFrameContext());

            hashTableInsert(table, "UFID", id3v2CreateUniqueFileIdentifierFrameContext());
            hashTableInsert(table, "USER", id3v2CreateTermsOfUseFrameContext());
            hashTableInsert(table, "USLT", id3v2CreateUnsynchronisedLyricFrameContext());

            hashTableInsert(table, "WCOM", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WCOP", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WOAF", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WOAR", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WOAS", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WORS", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WPAY", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WPUB", id3v2CreateURLFrameContext());
            hashTableInsert(table, "WXXX", id3v2CreateUserDefinedURLFrameContext());
            
            break;
        default:
            break;
    }

    hashTableInsert(table, "?", id3v2CreateGenericFrameContext());

    return table;
}

/**
 * @brief Inserts a new frame ID and context pairing into a map
 * 
 * @param identifierContextPairs 
 * @param key 
 * @param context 
 * @return true 
 * @return false 
 */
bool id3v2InsertIdentifierContextPair(HashTable *identifierContextPairs, char key[ID3V2_FRAME_ID_MAX_SIZE], List *context){

    if(!identifierContextPairs || !key || !context){
        return false;
    }

    hashTableInsert(identifierContextPairs, key, context);

    return true;
}

