
#include <assert.h>
#include <stdlib.h>
#include <baselib/baselib.h>

#include "xml_attribute.h"
#include "xml_attribute_struct.h"
#include "xml_writer.h"

#include "xml_element.h"
#include "xml_element_struct.h"


static void xml_element_destroy_attribute_any(Any any)
{
  xml_attribute_destroy((XmlAttribute *) any_to_ptr(any));
}

static void xml_element_destroy_child_any(Any any)
{
  if (any.type == ANY_TYPE_STRING)
    free(any_to_string(any));
  else
    xml_element_destroy((XmlElement *) any_to_ptr(any));
}




XmlElement * xml_element_new(char * name)
{
  assert(name);

  XmlElement * ret = (XmlElement *) malloc(sizeof(XmlElement));
  assert(ret);

  ret->name = strings_clone(name);
  ret->attributes = array_list_new();
  ret->children = array_list_new();

  return ret;
}

XmlElement * xml_element_parse(char * string)
{
  assert(0);
}


void xml_element_destroy(XmlElement * element)
{
  assert(element);

  array_list_destroy_and(element->attributes, xml_element_destroy_attribute_any);
  array_list_destroy_and(element->children, xml_element_destroy_child_any);
  free(element->name);

  free(element);
}



char * xml_element_get_name(XmlElement * element)
{
  assert(element);

  return strings_clone(element->name);
}

List * xml_element_get_attributes(XmlElement * element)
{
  assert(element);

  return (List *) array_list_clone(element->attributes);
}

List * xml_element_get_children(XmlElement * element)
{
  assert(element);

  return (List *) array_list_clone(element->children);
}

bool xml_element_is_empty(XmlElement * element)
{
  assert(element);

  return array_list_size(element->children) == 0;
}


XmlAttribute * xml_element_get_attribute(XmlElement * element, char * name)
{
  assert(element);
  assert(name);

  XmlAttribute * attrib;
  ArrayListTraversal * traversal = array_list_get_traversal(element->attributes);
  while (!array_list_traversal_completed(traversal))
  {

    attrib = (XmlAttribute *) any_to_ptr(array_list_traversal_next(traversal));

    if (strings_equals(attrib->name, name))
    {
      array_list_traversal_destroy(traversal);
      return attrib;
    }

  }

  return NULL;
}

XmlElement * xml_element_get_child(XmlElement * element, char * name)
{
  assert(element);
  assert(name);

  Any child_any;
  XmlElement * child;
  ArrayListTraversal * traversal = array_list_get_traversal(element->children);
  while (!array_list_traversal_completed(traversal))
  {
    child_any = array_list_traversal_next(traversal);
    if (child_any.type != ANY_TYPE_POINTER)
      continue;
    child = (XmlElement *) any_to_ptr(child_any);

    if (strings_equals(child->name, name))
    {
      array_list_traversal_destroy(traversal);

      return child;
    }
  }

  return NULL;
}

char * xml_element_get_value(XmlElement * element)
{
  XmlWriter * writer;
  ArrayListTraversal * traversal;
  StringBuilder * sb;

  traversal = array_list_get_traversal(element->children);
  sb = string_builder_new();

  writer = xml_writer_new();
  xml_writer_set_style(writer, XML_WRITER_STYLE_COMPRESSED);

  while (!array_list_traversal_completed(traversal))
  {
    Any child_any = array_list_traversal_next(traversal);
    if (child_any.type == ANY_TYPE_STRING)
    {
      string_builder_append(sb, any_to_string(child_any));
    }
    else if (child_any.type == ANY_TYPE_POINTER)
    {
      char * string = xml_writer_get_element_text(writer, (XmlElement *) any_to_ptr(child_any));
      string_builder_append(sb, string);
      free(string);
    }
    else
      assert(0);
  }

  xml_writer_destroy(writer);

  return string_builder_to_string_destroy(sb);
}

void xml_element_set_name(XmlElement * element, char * name)
{
  assert(element);
  assert(name);

  element->name = strings_clone(name);
}

void xml_element_set_value(XmlElement * element, char * value)
{
  array_list_clear_and(element->children, xml_element_destroy_child_any);
  array_list_add(element->children, string_to_any(strings_clone(value)));
}


void xml_element_clear_attributes(XmlElement * element)
{
  assert(element);

  array_list_clear_and(element->attributes, xml_element_destroy_attribute_any);
}

void xml_element_clear_children(XmlElement * element)
{
  assert(element);

  array_list_clear_and(element->attributes, xml_element_destroy_child_any);
}


void xml_element_add_attribute(XmlElement * element, XmlAttribute * attribute)
{
  assert(element);
  assert(attribute);

  array_list_add(element->attributes, ptr_to_any(attribute));
}

void xml_element_add_child(XmlElement * element, XmlElement * child)
{
  assert(element);
  assert(child);

  array_list_add(element->children, ptr_to_any(child));
}


void xml_element_add_attributes(XmlElement * element, List * attributes)
{
  assert(element);
  assert(attributes);

  array_list_add_range(element->attributes, attributes);
}

void xml_element_add_children(XmlElement * element, List * children)
{
  assert(element);
  assert(children);

  array_list_add_range(element->children, children);
}
