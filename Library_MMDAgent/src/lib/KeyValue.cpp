/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* headers */

#include <stdarg.h>
#include "MMDAgent.h"

#define KEYVALUE_MAXNUM           4096

/* KeyValue::initialize: initialize data */
void KeyValue::initialize()
{
   m_data = NULL;
   m_index = NULL;
   m_env = NULL;
   m_num = 0;
}

/* KeyValue::clear: free data */
void KeyValue::clear()
{
   int i;

   if (m_data != NULL) {
      for (i = 0; i < m_num; i++) {
         if (m_data[i] != NULL)
            free(m_data[i]);
      }
      free(m_data);
   }
   if (m_index != NULL)
      delete m_index;
   if (m_env != NULL)
      delete m_env;
   initialize();
}

/* KeyValue::KeyValue: constructor */
KeyValue::KeyValue()
{
   initialize();
}

/* KeyValue::~KeyValue: destructor */
KeyValue::~KeyValue()
{
   clear();
}

/* KeyValue::setup: setup */
void KeyValue::setup()
{
   clear();
   m_data = (char **) malloc(sizeof(char *) * KEYVALUE_MAXNUM);
   m_index = new PTree;
}

/* KeyValue::loadBuf: load "key=value" pairs and set values from buffer */
bool KeyValue::loadBuf(const char *buffer)
{
   char buf[MMDAGENT_MAXBUFLEN];
   int len;
   char *p1;

   MMDAgent_snprintf(buf, MMDAGENT_MAXBUFLEN, "%s", buffer);
   len = MMDAgent_strlen(buf);
   if (len <= 0)
      return false;
   p1 = &(buf[len - 1]);
   while (p1 >= &(buf[0]) && (*p1 == '\n' || *p1 == '\r' || *p1 == '\t' || *p1 == ' ')) {
      *p1 = L'\0';
      p1--;
   }
   p1 = &(buf[0]);
   if (*p1 == '#')
      return false;
   while (*p1 != L'=' && *p1 != L'\0') p1++;
   if (*p1 == L'\0')
      return false;
   *p1 = L'\0';
   p1++;
   setString(buf, "%s", p1);
   return true;
}

/* KeyValue::load: load "key=value" pairs in a file and set values */
bool KeyValue::load(const char *file, ZFileKey *key)
{
   char buf[MMDAGENT_MAXBUFLEN];
   ZFile *zf;

   zf = new ZFile(key);
   if (zf->openAndLoad(file) == false) {
      delete zf;
      return false;
   }
   while (zf->gets(buf, MMDAGENT_MAXBUFLEN))
      loadBuf(buf);
   zf->close();
   delete zf;
   return true;
}

/* KeyValue::save: save "key=value" pairs to a file */
bool KeyValue::save(const char *file)
{
   char buff[MMDAGENT_MAXBUFLEN];
   FILE *fp;
   void *save;
   const char *key;

   fp = MMDAgent_fopen(file, "w");
   if (fp == NULL)
      return false;
   for (key = m_index->firstKey(&save); key; key = m_index->nextKey(&save)) {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s=%s\n", key, getString(key, ""));
      if (fputs(buff, fp) < 0) {
         fclose(fp);
         return false;
      }
   }
   fclose(fp);
   return true;
}

/* KeyValue::loadText: load "key\nvalue\n" pairs in a file and set values */
bool KeyValue::loadText(const char *file, ZFileKey *key, bool append)
{
   char buff1[MMDAGENT_MAXBUFLEN];
   char buff2[MMDAGENT_MAXBUFLEN];
   ZFile *zf;
   bool sw;
   int len;
   char *p;

   zf = new ZFile(key);
   if (zf->openAndLoad(file) == false) {
      delete zf;
      return false;
   }
   sw = false;
   while (zf->gets(buff1, MMDAGENT_MAXBUFLEN)) {
      len = MMDAgent_strlen(buff1);
      if (len <= 0) {
         sw = false;
         continue;
      }
      p = &(buff1[len - 1]);
      while (p >= &(buff1[0]) && (*p == '\n' || *p == '\r')) {
         *p = L'\0';
         p--;
      }
      if (sw == false) {
         strcpy(buff2, buff1);
         sw = true;
      } else {
         if (append == false || exist(buff2) == false)
            setString(buff2, "%s", buff1);
         sw = false;
      }
   }
   zf->close();
   delete zf;
   return true;

}

/* KeyValue::saveText: save "key\nvalue\n" pairs to a file */
bool KeyValue::saveText(const char *file)
{
   char buff[MMDAGENT_MAXBUFLEN];
   FILE *fp;
   void *save;
   const char *key;

   fp = MMDAgent_fopen(file, "w");
   if (fp == NULL)
      return false;
   for (key = m_index->firstKey(&save); key; key = m_index->nextKey(&save)) {
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s\n", key);
      if (fputs(buff, fp) < 0) {
         fclose(fp);
         return false;
      }
      MMDAgent_snprintf(buff, MMDAGENT_MAXBUFLEN, "%s\n", getString(key, ""));
      if (fputs(buff, fp) < 0) {
         fclose(fp);
         return false;
      }
   }
   fclose(fp);
   return true;
}

/* KeyValue::exist: check if the key exists */
bool KeyValue::exist(const char *key)
{
   if (findData(key) != NULL)
      return true;
   return false;
}

/* KeyValue::find: find data */
char **KeyValue::findData(const char *key)
{
   char **data;

   if (key == NULL)
      return NULL;
   if (m_index == NULL)
      return NULL;
   if (m_index->search(key, MMDAgent_strlen(key), (void **) &data) == false)
      return NULL;
   return data;
}

/* KeyValue::setString: set string value */
bool KeyValue::setString(const char *key, const char *format, ...)
{
   char buff[MMDAGENT_MAXBUFLEN];
   int len;
   char **data;
   va_list args;

   if (m_index == NULL)
      return false;

   len = MMDAgent_strlen(key);
   if (m_index->search(key, len, (void **)&data) == false) {
      if (m_num >= KEYVALUE_MAXNUM)
         return false;
      data = &(m_data[m_num]);
      *data = NULL;
      m_index->add(key, len, (void *)data);
      m_num++;
   }

   va_start(args, format);
   vsprintf(buff, format, args);
   va_end(args);

   if (*data != NULL)
      free(*data);
   *data = MMDAgent_strdup(buff);

   return true;
}

/* KeyValue::getString: get string value */
const char *KeyValue::getString(const char *key, const char *default_value)
{
   char **data;
   const char *str;
   char buf[MMDAGENT_MAXBUFLEN];
   int ret;

   data = findData(key);

   str = (data != NULL) ? *data : default_value;

   if (str == NULL)
      return NULL;

   /* consult env value embedding */
   ret = MMDAgent_replaceEnvDup(str, buf);
   if (ret < 0) {
      /* error, return original indexed string */
      return str;
   }

   /* env has been embedded, keep buffer */
   char **dataEnv;
   int len;
   if (m_env == NULL)
      m_env = new PTree;
   len = MMDAgent_strlen(key);
   if (m_env->search(key, len, (void **)&dataEnv) == false) {
      if (m_num >= KEYVALUE_MAXNUM)
         return NULL;
      dataEnv = &(m_data[m_num]);
      *dataEnv = (char *)malloc(MMDAGENT_MAXBUFLEN);
      m_env->add(key, len, (void *)dataEnv);
      m_num++;
   }
   memcpy(*dataEnv, buf, MMDAGENT_MAXBUFLEN);

   return *dataEnv;
}

/* KeyValue::firstKey: get first key */
const char *KeyValue::firstKey(void **save)
{
   if (m_index == NULL)
      return NULL;
   return m_index->firstKey(save);
}

/* KeyValue::nextKey: get next key */
const char *KeyValue::nextKey(void **save)
{
   if (m_index == NULL)
      return NULL;
   return m_index->nextKey(save);
}
