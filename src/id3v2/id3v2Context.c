#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "LinkedList.h"
#include "id3v2Context.h"
#include "byteStream.h"
#include "byteInt.h"

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
 * @brief deletes a list of contexts
 * @details used for the hashtable library
 * @param toBeDeleted 
 */
void id3v2DeleteContentContextList(void *toBeDeleted){
    listFree((List *)toBeDeleted);
}

/**
 * @brief returns a char * representation of a context list
 * @details used for the hashtable library
 * @param toBePrinted 
 * @return int 
 */
char *id3v2PrintContentContextList(const void *toBePrinted){
    return listToString((List *)toBePrinted);
}

/**
 * @brief copys a context list
 * @details used for the hashtable library
 * @param toBeCopied 
 * @return void* 
 */
void *id3v2CopyContentContextList(const void *toBeCopied){
    return listDeepCopy((List *)toBeCopied);
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

    size_t minFrameContexts = 66;
    HashTable *table = hashTableCreate(minFrameContexts, id3v2DeleteContentContextList, id3v2PrintContentContextList, id3v2CopyContentContextList);
    List *l = NULL;

    switch(version){
        
        case ID3V2_TAG_VERSION_2:
            
            hashTableInsert(table, "BUF", (l = id3v2CreateRecommendedBufferSizeFrameContext()));
            listFree(l);

            hashTableInsert(table, "CNT", (l = id3v2CreatePlayCounterFrameContext()));
            listFree(l);
            
            hashTableInsert(table, "COM", (l = id3v2CreateCommentFrameContext()));
            listFree(l);
            hashTableInsert(table, "CRA", (l = id3v2CreateAudioEncryptionFrameContext()));
            listFree(l);
            hashTableInsert(table, "CRM", (l = id3v2CreateEncryptedMetaFrameContext()));
            listFree(l);

            hashTableInsert(table, "ETC", (l = id3v2CreateEventTimingCodesFrameContext()));
            listFree(l);
            hashTableInsert(table, "EQU", (l = id3v2CreateEqulizationFrameContext(version)));
            listFree(l);

            hashTableInsert(table, "GEO", (l = id3v2CreateGeneralEncapsulatedObjectFrameContext()));
            listFree(l);

            hashTableInsert(table, "IPL", (l = id3v2CreateInvolvedPeopleListFrameContext()));
            listFree(l);

            hashTableInsert(table, "LNK", (l = id3v2CreateLinkedInformationFrameContext()));
            listFree(l);

            hashTableInsert(table, "MCI", (l = id3v2CreateMusicCDIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "MLL", (l = id3v2CreateMPEGLocationLookupTableFrameContext()));
            listFree(l);

            hashTableInsert(table, "PIC", (l = id3v2CreateAttachedPictureFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "POP", (l = id3v2CreatePopularimeterFrameContext()));
            listFree(l);

            hashTableInsert(table, "REV", (l = id3v2CreateReverbFrameContext()));
            listFree(l);
            hashTableInsert(table, "RVA", (l = id3v2CreateRelativeVolumeAdjustmentFrameContext(version)));
            listFree(l);

            hashTableInsert(table, "SLT", (l = id3v2CreateSynchronisedLyricFrameContext()));
            listFree(l);
            hashTableInsert(table, "STC", (l = id3v2CreateSynchronisedTempoCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "TAL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TBP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCR", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TEN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TFT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TKE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOF", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOR", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TP4", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRD", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRK", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSI", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSS", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TT1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TT2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TT3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TXT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TXX", (l = id3v2CreateUserDefinedTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TYE", (l = id3v2CreateTextFrameContext()));
            listFree(l);

            hashTableInsert(table, "UFI", (l = id3v2CreateUniqueFileIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "ULT", (l = id3v2CreateUnsynchronisedLyricFrameContext()));
            listFree(l);

            hashTableInsert(table, "WAF", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WAR", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WAS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCM", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCP", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPB", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WXX", (l = id3v2CreateUserDefinedURLFrameContext()));
            listFree(l);

            break;
        case ID3V2_TAG_VERSION_3:

            hashTableInsert(table, "AENC", (l = id3v2CreateAudioEncryptionFrameContext()));
            listFree(l);
            hashTableInsert(table, "APIC", (l = id3v2CreateAttachedPictureFrameContext(version)));
            listFree(l);

            hashTableInsert(table, "COMM", (l = id3v2CreateCommentFrameContext()));
            listFree(l);
            hashTableInsert(table, "COMR", (l = id3v2CreateCommercialFrameContext()));
            listFree(l);

            hashTableInsert(table, "ENCR", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);
            hashTableInsert(table, "EQUA", (l = id3v2CreateEqulizationFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "ETCO", (l = id3v2CreateEventTimingCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "GEOB", (l = id3v2CreateGeneralEncapsulatedObjectFrameContext()));
            listFree(l);
            hashTableInsert(table, "GRID", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);

            hashTableInsert(table, "IPLS", (l = id3v2CreateInvolvedPeopleListFrameContext()));
            listFree(l);

            hashTableInsert(table, "LINK", (l = id3v2CreateLinkedInformationFrameContext()));
            listFree(l);
            
            hashTableInsert(table, "MCDI", (l = id3v2CreateMusicCDIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "MLLT", (l = id3v2CreateMPEGLocationLookupTableFrameContext()));
            listFree(l);
            
            hashTableInsert(table, "OWNE", (l = id3v2CreateOwnershipFrameContext()));
            listFree(l);
            
            hashTableInsert(table, "PRIV", (l = id3v2CreatePrivateFrameContext()));
            listFree(l);
            hashTableInsert(table, "PCNT", (l = id3v2CreatePlayCounterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POPM", (l = id3v2CreatePopularimeterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POSS", (l = id3v2CreatePositionSynchronisationFrameContext()));
            listFree(l);

            hashTableInsert(table, "RBUF", (l = id3v2CreateRecommendedBufferSizeFrameContext()));
            listFree(l);
            hashTableInsert(table, "RVAD", (l = id3v2CreateRelativeVolumeAdjustmentFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "RVRB", (l = id3v2CreateReverbFrameContext()));
            listFree(l);

            hashTableInsert(table, "SYLT", (l = id3v2CreateSynchronisedLyricFrameContext()));
            listFree(l);
            hashTableInsert(table, "SYTC", (l = id3v2CreateSynchronisedTempoCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "TALB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TBPM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOM", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TCON", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDAT", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TDLY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TENC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TEXT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TFLT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIME", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TIT1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TKEY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLAN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLEN", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TMED", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TOAL", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TOFN", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TOLY", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TOPE", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TORY", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TOWN", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TPE1", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TPE2", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TPE3", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TPE4", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TPOS", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TPUB", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TRCK", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TRDA", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TRSN", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TRSO", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TSIZ", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TSRC", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TSSE", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TYER", (l = id3v2CreateTextFrameContext()));
            listFree(l); 
            hashTableInsert(table, "TXXX", (l = id3v2CreateUserDefinedTextFrameContext()));
            listFree(l);

            hashTableInsert(table, "UFID", (l = id3v2CreateUniqueFileIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "USER", (l = id3v2CreateTermsOfUseFrameContext()));
            listFree(l);
            hashTableInsert(table, "USLT", (l = id3v2CreateUnsynchronisedLyricFrameContext()));
            listFree(l);

            hashTableInsert(table, "WCOM", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCOP", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAF", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAR", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WORS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPAY", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPUB", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WXXX", (l = id3v2CreateUserDefinedURLFrameContext()));
            listFree(l);

            break;
        case ID3V2_TAG_VERSION_4:

            hashTableInsert(table, "AENC", (l = id3v2CreateAudioEncryptionFrameContext()));
            listFree(l);
            hashTableInsert(table, "APIC", (l = id3v2CreateAttachedPictureFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "ASPI", (l = id3v2CreateAudioSeekPointIndexFrameContext()));
            listFree(l);

            hashTableInsert(table, "COMM", (l = id3v2CreateCommentFrameContext()));
            listFree(l);
            hashTableInsert(table, "COMR", (l = id3v2CreateCommercialFrameContext()));
            listFree(l);

            hashTableInsert(table, "ENCR", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);
            hashTableInsert(table, "EQU2", (l = id3v2CreateEqulizationFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "ETCO", (l = id3v2CreateEventTimingCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "GEOB", (l = id3v2CreateGeneralEncapsulatedObjectFrameContext()));
            listFree(l);
            hashTableInsert(table, "GRID", (l = id3v2CreateRegistrationFrameContext()));
            listFree(l);

            hashTableInsert(table, "LINK", (l = id3v2CreateLinkedInformationFrameContext()));
            listFree(l);

            hashTableInsert(table, "MCDI", (l = id3v2CreateMusicCDIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "MLLT", (l = id3v2CreateMPEGLocationLookupTableFrameContext()));
            listFree(l);

            hashTableInsert(table, "OWNE", (l = id3v2CreateOwnershipFrameContext()));
            listFree(l);

            hashTableInsert(table, "PRIV", (l = id3v2CreatePrivateFrameContext()));
            listFree(l);
            hashTableInsert(table, "PCNT", (l = id3v2CreatePlayCounterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POPM", (l = id3v2CreatePopularimeterFrameContext()));
            listFree(l);
            hashTableInsert(table, "POSS", (l = id3v2CreatePositionSynchronisationFrameContext()));
            listFree(l);

            hashTableInsert(table, "RBUF", (l = id3v2CreateRecommendedBufferSizeFrameContext()));
            listFree(l);
            hashTableInsert(table, "RVA2", (l = id3v2CreateRelativeVolumeAdjustmentFrameContext(version)));
            listFree(l);
            hashTableInsert(table, "RVRB", (l = id3v2CreateReverbFrameContext()));
            listFree(l);

            hashTableInsert(table, "SEEK", (l = id3v2CreateSeekFrameContext()));
            listFree(l);
            hashTableInsert(table, "SIGN", (l = id3v2CreateSignatureFrameContext()));
            listFree(l);
            hashTableInsert(table, "SYLT", (l = id3v2CreateSynchronisedLyricFrameContext()));
            listFree(l);
            hashTableInsert(table, "SYTC", (l = id3v2CreateSynchronisedTempoCodesFrameContext()));
            listFree(l);

            hashTableInsert(table, "TALB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TBPM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOM", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCON", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TCOP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDEN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDLY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDOR", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDRC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDRL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TDTG", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TENC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TEXT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TFLT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIPL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TIT3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TKEY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLAN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TLEN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMCL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMED", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TMOO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOAL", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOFN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOLY", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOPE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TOWN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE1", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE2", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE3", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPE4", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPOS", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPRO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TPUB", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRCK", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRSN", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TRSO", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSOA", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSOP", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSOT", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSRC", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSSE", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TSST", (l = id3v2CreateTextFrameContext()));
            listFree(l);
            hashTableInsert(table, "TXXX", (l = id3v2CreateUserDefinedTextFrameContext()));
            listFree(l);

            hashTableInsert(table, "UFID", (l = id3v2CreateUniqueFileIdentifierFrameContext()));
            listFree(l);
            hashTableInsert(table, "USER", (l = id3v2CreateTermsOfUseFrameContext()));
            listFree(l);
            hashTableInsert(table, "USLT", (l = id3v2CreateUnsynchronisedLyricFrameContext()));
            listFree(l);

            hashTableInsert(table, "WCOM", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WCOP", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAF", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAR", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WOAS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WORS", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPAY", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WPUB", (l = id3v2CreateURLFrameContext()));
            listFree(l);
            hashTableInsert(table, "WXXX", (l = id3v2CreateUserDefinedURLFrameContext()));
            listFree(l);
            
            break;
        default:
            break;
    }

    hashTableInsert(table, "?", (l = id3v2CreateGenericFrameContext()));
    listFree(l);
    hashTableInsert(table, "T", (l = id3v2CreateTextFrameContext()));
    listFree(l);
    hashTableInsert(table, "W", (l = id3v2CreateURLFrameContext()));
    listFree(l);

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

/**
 * @brief Creates a stream that represents the passed context
 * 
 * @param cc 
 * @return ByteStream* 
 */
ByteStream *id3v2ContextToStream(Id3v2ContentContext *cc){
    
    ByteStream *stream = NULL;
    size_t s = 0;
    unsigned char *convi = NULL;

    if(cc == NULL){
        return stream;
    }

    s += (sizeof(size_t) * 3) + 1;
    
    stream = byteStreamCreate(NULL, s);

    byteStreamWrite(stream, (uint8_t *) &cc->type, 1);

    convi = sttob(cc->key);
    byteStreamWrite(stream, convi, sizeof(size_t));
    free(convi);

    convi = sttob(cc->max);
    byteStreamWrite(stream, convi, sizeof(size_t));
    free(convi);

    convi = sttob(cc->min);
    byteStreamWrite(stream, convi, sizeof(size_t));
    free(convi);

    byteStreamRewind(stream);
    return stream;
}

char *id3v2ContextToJSON(Id3v2ContentContext *cc){
    
    char *json = NULL;
    size_t memCount = 3;
    if(cc == NULL){
        json = malloc(sizeof(char) * memCount);
        memcpy(json, "{}\0", memCount);
        return json;
    }

    memCount += (sizeof(size_t) * 3) + 1 + 28;

    json = malloc(sizeof(char) * memCount);
    memset(json, 0, memCount);

    sprintf(json, "{\"type\":%d,\"key\":%ld,\"max\":%ld,\"min\":%ld}", cc->type, cc->key, cc->max, cc->min);

    return json;
}