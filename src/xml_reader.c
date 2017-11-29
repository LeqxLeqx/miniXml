

#include <assert.h>
#include <baselib/baselib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "xml_element.h"
#include "xml_element_struct.h"
#include "xml_tokenizer.h"
#include "xml_token.h"

#include "xml_reader.h"


struct XmlReader
{
  List * tokens;
  Dictionary * entities;
  unsigned int tokens_current;
  
  char * error_message;
  char * returnable_error_message;
};

/* INTERNALS */

static void xml_reader_set_error_message(XmlReader * reader, char * error)
{
  reader->error_message = strings_clone(error);
}

static void xml_reader_load_default_entities(XmlReader * reader)
{
    reader->entities = dictionary_new(DICTIONARY_TYPE_HASH_TABLE);

    dictionary_put(reader->entities, "lt", str_to_any("<"));
    dictionary_put(reader->entities, "gt", str_to_any(">"));
    dictionary_put(reader->entities, "quot", str_to_any("\""));
    dictionary_put(reader->entities, "apos", str_to_any("\'"));
}

static void xml_reader_unload_entities(XmlReader * reader)
{
 
  dictionary_remove(reader->entities, "lt"); /* remove as not to free static data */
  dictionary_remove(reader->entities, "gt");
  dictionary_remove(reader->entities, "quot");
  dictionary_remove(reader->entities, "apos");

  dictionary_destroy_and_free(reader->entities);
}


static void xml_reader_reset(XmlReader * reader)
{
  if (reader->error_message)
    free(reader->error_message);
  if (reader->returnable_error_message)
    free(reader->returnable_error_message);
  
  reader->error_message = NULL;
  reader->returnable_error_message = NULL;
}


static XmlToken * xml_reader_current_token(XmlReader * reader)
{
  if (reader->tokens_current >= list_size(reader->tokens))
    return (XmlToken *) any_to_ptr(list_get(reader->tokens, list_size(reader->tokens) - 1));
  else
    return (XmlToken *) any_to_ptr(list_get(reader->tokens, reader->tokens_current));
}

static bool xml_reader_accept(XmlReader * reader, enum XmlTokenType type)
{
  XmlToken * token = xml_reader_current_token(reader);
  if (token->type == type)
  {
    reader->tokens_current++;
    return true;
  }
  else
    return false;
}

static void xml_reader_prepare_error_message(XmlReader * reader)
{
  if (reader->error_message)
  {
    char buffer [2048];
    sprintf(buffer, "%s (%s)", reader->error_message, xml_token_type_get_string(xml_reader_current_token(reader)->type));
    
    reader->returnable_error_message = strings_clone(buffer);
  }
}


static XmlElement * xml_reader_parse_element_imp(XmlReader * reader)
{
  XmlElement 
    * ret,
    * child;
  XmlToken
    * tag_name_token,
    * attrib_name_token,
    * attrib_value_token
    ;
  bool 
    inTokenLoop,
    tagEmpty;
  
  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_START_TAG))
  {
    xml_reader_set_error_message(reader, "Expected '<'");
    return NULL;
  }
  
  tag_name_token = xml_reader_current_token(reader);
  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_IDENTIFIER))
  {
    xml_reader_set_error_message(reader, "Expected tag name identifier");
    return NULL;
  }
  
  /* TODO: Confirm validity of tag name if neccessary */
  
  ret = xml_element_new(tag_name_token->data);   
  
  
  inTokenLoop = true;
  do
  {
    
    if (xml_reader_accept(reader, XML_TOKEN_TYPE_END_TAG))
    {
      tagEmpty = false;
      inTokenLoop = false;
      continue;
    }
    if (xml_reader_accept(reader, XML_TOKEN_TYPE_END_EMPTY_TAG))
    {
      tagEmpty = true;
      inTokenLoop = false;
      continue;
    }
    
    
    attrib_name_token = xml_reader_current_token(reader);
    
    if (!xml_reader_accept(reader, XML_TOKEN_TYPE_IDENTIFIER))
    {
      xml_reader_set_error_message(reader, "Expected attribute identifier");
      xml_element_destroy(ret);
      return NULL;
    }
    if (!xml_reader_accept(reader, XML_TOKEN_TYPE_EQUALS))
    {
      xml_reader_set_error_message(reader, "Expected '='");
      xml_element_destroy(ret);
      return NULL;
    }
    attrib_value_token = xml_reader_current_token(reader);
    if (!xml_reader_accept(reader, XML_TOKEN_TYPE_QUOTED_STRING))
    {
      xml_reader_set_error_message(reader, "Expected quoted attribute value");
      xml_element_destroy(ret);
      return NULL;
    }
    
    xml_element_add_attribute(
      ret, 
      xml_attribute_new(
        attrib_name_token->data,
        attrib_value_token->data
        )
      );
  }
  while (inTokenLoop);
  
  if (tagEmpty)
    return ret;
  
  
  while (
    !xml_reader_accept(reader, XML_TOKEN_TYPE_START_END_TAG)
    )
  {
    
    XmlToken * current = xml_reader_current_token(reader);
    
    switch (current->type)
    {
      case XML_TOKEN_TYPE_TEXT:
        array_list_add(ret->children, str_to_any(strings_clone(current->data)));
        reader->tokens_current++;
        break;
      case XML_TOKEN_TYPE_START_TAG:
        child = xml_reader_parse_element_imp(reader);
        if (!child)
        {
          xml_element_destroy(ret);
          return NULL;
        }
        array_list_add(ret->children, void_to_any(child));
        break;
      default:
        xml_reader_set_error_message(reader, "Unexpected token");
        xml_element_destroy(ret);
        return NULL;
    }
  }

  XmlToken * close_identifier = xml_reader_current_token(reader);
  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_IDENTIFIER))
  {
    xml_reader_set_error_message(reader, "Unexpected token");
    return NULL;
  }
  
  if (!strings_equals(close_identifier->data, tag_name_token->data))
  {
    xml_reader_set_error_message(reader, "Close tag does not match open tag");
    return NULL;
  }
  
  if (!xml_reader_accept(reader, XML_TOKEN_TYPE_END_TAG))
  {
    xml_reader_set_error_message(reader, "Unexpected token");
    return NULL;
  }
  
  
  return ret;
}




XmlReader * xml_reader_new()
{
  XmlReader * reader = (XmlReader *) malloc(sizeof(XmlReader));
  assert(reader);

  reader->error_message = NULL;
  reader->returnable_error_message = NULL;

  return reader;
}

void xml_reader_destroy(XmlReader * reader)
{
  assert(reader);

  if (reader->error_message)
    free(reader->error_message);
  if (reader->returnable_error_message)
    free(reader->returnable_error_message);
  
  free(reader);
}


char * xml_reader_get_error_message(XmlReader * reader)
{
  assert(reader);
  
  return strings_clone(reader->returnable_error_message);
}

XmlDocument * xml_reader_parse_document(XmlReader * reader, char * data, size_t data_length)
{
  assert(reader);
  assert(data);
  assert(data_length > 0); 
  
  assert(0); /* NOT YET IMPLEMENTED */
  
}

XmlElement * xml_reader_parse_element(XmlReader * reader, char * data)
{
  assert(reader);
  assert(data);
  
  xml_reader_reset(reader);

  
  XmlTokenizer * tokenizer = xml_tokenizer_new(data);
  LinkedList * tokens = xml_tokenizer_tokenize(tokenizer);
  
  if (!tokens)
  {
    char buffer [1024];
    if (strings_contains_string(tokenizer->error_message, "%c"))
    {
      char * concat = strings_concat(tokenizer->error_message, " [%02x] (line: %d, column: %d)");
      char c = xml_tokenizer_character(tokenizer);
      sprintf(buffer, concat, c, c, tokenizer->line, tokenizer->column);
      free(concat);
    }
    else
      sprintf(buffer, "%s (line: %d, column: %d)", tokenizer->error_message, tokenizer->line, tokenizer->column);
    
    
    xml_reader_set_error_message(reader, buffer);
    return NULL;
  }
  else
  {
    
    reader->tokens = (List *) tokens;
    reader->tokens_current = 0;

    xml_reader_load_default_entities(reader);
      
    /* Add end of file token */
    list_add(reader->tokens, ptr_to_any(xml_token_new(XML_TOKEN_TYPE_END_OF_FILE, NULL)));
    
    XmlElement * element = xml_reader_parse_element_imp(reader);
    
    xml_reader_prepare_error_message(reader);
    
    list_destroy_and_user_free(reader->tokens, (void (*)(void *)) xml_token_destroy);
    xml_reader_unload_entities(reader);
    
    return element;
  }
  
}
