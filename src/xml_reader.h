/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  miniXml: a simple XML parsing library for C                            *
 *  Copyright (C) 2017  LeqxLeqx                                           *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

 
#ifndef __XML_READER_H
#define __XML_READER_H


#include "xml_document.h"
#include "xml_document_struct.h"
#include "xml_element.h"
#include "xml_element_struct.h"
#include "xml_attribute.h"
#include "xml_attribute_struct.h"


struct XmlReader;
typedef struct XmlReader XmlReader;


XmlReader * xml_reader_new();
void xml_reader_destroy(XmlReader * reader);

char * xml_reader_get_error_message(XmlReader * reader);

/* CAN BE ENCODED IN ANY MANNER WHICH IS ACCEPTABLE */
XmlDocument * xml_reader_parse_document(XmlReader * reader, char * data, size_t data_size);

/* EXPECTS UTF-8 */
XmlElement * xml_reader_parse_element(XmlReader * reader, char * data);




#endif
