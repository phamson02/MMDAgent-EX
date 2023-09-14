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

#include "MMDAgent.h"

/* ShapeMap::initialize: initialize */
void ShapeMap::initialize()
{
   m_eyeRotationCoef = 1.0f;
   m_lipMorph = NULL;
   m_trackBone = NULL;
   m_auMorph = NULL;
   m_arKitMorph = NULL;
   m_actionMotion = NULL;
   m_exBone = NULL;
   m_exMorph = NULL;
   m_morphTuneNum = 0;
   m_lipIgnoreList = NULL;
}

/* ShapeMap::freeMorphInstance: free morph instance in a tree */
void ShapeMap::freeMorphInstance(PTree *tree)
{
   PMDFaceInterface *f;
   void *save;
   for (f = (PMDFaceInterface *)tree->firstData(&save); f; f = (PMDFaceInterface *)tree->nextData(&save))
      delete f;
}

/* ShapeMap::clear: free */
void ShapeMap::clear()
{
   if (m_lipMorph) {
      freeMorphInstance(m_lipMorph);
      delete m_lipMorph;
   }
   if (m_trackBone)
      delete m_trackBone;
   if (m_auMorph) {
      freeMorphInstance(m_auMorph);
      delete m_auMorph;
   }
   if (m_arKitMorph) {
      freeMorphInstance(m_arKitMorph);
      delete m_arKitMorph;
   }
   if (m_actionMotion) {
      char *p;
      void *save;
      for (p = (char *)m_actionMotion->firstData(&save); p; p = (char *)m_actionMotion->nextData(&save))
         free(p);
   }
   delete m_actionMotion;
   if (m_exBone)
      delete m_exBone;
   if (m_exMorph) {
      freeMorphInstance(m_exMorph);
      delete m_exMorph;
   }

   for (int i = 0; i < m_morphTuneNum; i++) {
      for (int j = 0; j < m_morphTuneLen[i]; j++) {
         delete m_morphTuneSet[i][j];
      }
   }

   if (m_lipIgnoreList)
      free(m_lipIgnoreList);

   initialize();
}

/* ShapeMap::ShapeMap: constructor */
ShapeMap::ShapeMap()
{
   initialize();
}

/* ShapeMap::~ShapeMap: destructor */
ShapeMap::~ShapeMap()
{
   clear();
}

/* ShapeMap::load: load shape mapping information from file */
bool ShapeMap::load(const char *fileName, PMDModel *pmd, const char *dirName)
{
   ZFile *zf;
   char buf[MMDAGENT_MAXBUFLEN];
   char buf2[MMDAGENT_MAXBUFLEN];
   char *p1, *p2, *p3, *psave;

   if (fileName == NULL)
      return false;

   if (MMDAgent_exist(fileName)) {
      zf = new ZFile(g_enckey);
      if (zf->openAndLoad(fileName)) {
         clear();
         m_lipMorph = new PTree;
         m_trackBone = new PTree;
         m_auMorph = new PTree;
         m_arKitMorph = new PTree;
         m_actionMotion = new PTree;
         m_exBone = new PTree;
         m_exMorph = new PTree;
         /* parse */
         while (zf->gets(buf, MMDAGENT_MAXBUFLEN) != NULL) {
            p1 = MMDAgent_strtok(buf, " \r\n", &psave);
            if (p1 == NULL) continue;
            if (p1[0] == '#') continue;
            p2 = MMDAgent_strtok(NULL, " \r\n", &psave);
            if (p2 == NULL) continue;
            if (MMDAgent_strheadmatch(p1, "EYE_ROTATION_COEF")) {
               m_eyeRotationCoef = MMDAgent_str2float(p2);
            } else if (MMDAgent_strheadmatch(p1, "LIP_")) {
               // lip sync
               PMDFaceInterface *f = getLipMorph(p1);
               if (f == NULL) {
                  f = new PMDFaceInterface(pmd, p2, 1.0f, false);
                  if (f->isValid())
                     m_lipMorph->add(p1, MMDAgent_strlen(p1), (void *)f);
                  else
                     delete f;
               }
            } else if (MMDAgent_strheadmatch(p1, "NOLIP")) {
               // lip ignore list
               if (m_lipIgnoreList)
                  free(m_lipIgnoreList);
               m_lipIgnoreList = MMDAgent_strdup(p2);
            } else if (MMDAgent_strheadmatch(p1, "TRACK_")) {
               // tracking
               PMDBone *btmp = getTrackBone(p1);
               if (btmp == NULL) {
                  char *save;
                  char *token;
                  strcpy(buf2, p2);
                  for (token = MMDAgent_strtok(buf2, ",\r\n", &save); token; token = MMDAgent_strtok(NULL, ",\r\n", &save)) {
                     PMDBone *b = pmd->getBone(token);
                     if (b) {
                        m_trackBone->add(p1, MMDAgent_strlen(p1), (void *)b);
                        break;
                     }
                  }
               }
            } else if (MMDAgent_strheadmatch(p1, "AU")) {
               // AU morph
               p3 = MMDAgent_strtok(NULL, " \r\n", &psave);
               float coef = 1.0f;
               bool thres = false;
               if (p3) {
                  if (p3[0] == '>') {
                     thres = true;
                     coef = MMDAgent_str2float(&(p3[1]));
                  } else {
                     thres = false;
                     coef = MMDAgent_str2float(p3);
                  }
               }
               PMDFaceInterface *f = getAUMorph(p1);
               if (f) {
                  f->add(pmd, p2, coef, thres);
               } else {
                  f = new PMDFaceInterface(pmd, p2, coef, thres);
                  if (f->isValid())
                     m_auMorph->add(p1, MMDAgent_strlen(p1), (void *)f);
                  else
                     delete f;
               }
            } else if (MMDAgent_strheadmatch(p1, "ACT")) {
               // action motion
               const char *p = getActionMotionFileName(p1);
               if (p == NULL) {
                  MMDAgent_snprintf(buf2, MMDAGENT_MAXBUFLEN, "%s%c%s", dirName, MMDFILES_DIRSEPARATOR, p2);
                  m_actionMotion->add(p1, MMDAgent_strlen(p1), MMDAgent_strdup(buf2));
               }
            } else if (MMDAgent_strheadmatch(p1, "EXBONE_")) {
               // ex bone
               PMDBone *btmp = getExBone(p1);
               if (btmp == NULL) {
                  char *save;
                  char *token;
                  strcpy(buf2, p2);
                  for (token = MMDAgent_strtok(buf2, ",\r\n", &save); token; token = MMDAgent_strtok(NULL, ",\r\n", &save)) {
                     PMDBone *b = pmd->getBone(token);
                     if (b) {
                        m_exBone->add(p1, MMDAgent_strlen(p1), (void *)b);
                        break;
                     }
                  }
               }
            } else if (MMDAgent_strheadmatch(p1, "EXMORPH_")) {
               // ex morph
               p3 = MMDAgent_strtok(NULL, " \r\n", &psave);
               float coef = 1.0f;
               bool thres = false;
               if (p3) {
                  if (p3[0] == '>') {
                     thres = true;
                     coef = MMDAgent_str2float(&(p3[1]));
                  } else {
                     thres = false;
                     coef = MMDAgent_str2float(p3);
                  }
               }
               PMDFaceInterface *f = getExMorph(p1);
               if (f) {
                  f->add(pmd, p2, coef, thres);
               } else {
                  f = new PMDFaceInterface(pmd, p2, coef, thres);
                  if (f->isValid())
                     m_exMorph->add(p1, MMDAgent_strlen(p1), (void *)f);
                  else
                     delete f;
               }
            } else if (MMDAgent_strheadmatch(p1, "MORPH_TUNE")) {
               // Morph tuning
               if (m_morphTuneNum < SHAPEMAP_MORPH_TUNE_MAXNUM) {
                  int n = 0;
                  do {
                     PMDFaceInterface *f = new PMDFaceInterface(pmd, p2, 1.0f, false);
                     if (f->isValid() && n < SHAPEMAP_MORPH_TUNE_MAXLEN) {
                        m_morphTuneSet[m_morphTuneNum][n] = f;
                        n++;
                     } else {
                        delete f;
                     }
                  } while ((p2 = MMDAgent_strtok(NULL, " \r\n", &psave)));
                  m_morphTuneLen[m_morphTuneNum] = n;
                  m_morphTuneNum++;
               }
            } else {
               // ARKit shapes
               p3 = MMDAgent_strtok(NULL, " \r\n", &psave);
               float coef = 1.0f;
               bool thres = false;
               if (p3) {
                  if (p3[0] == '>') {
                     thres = true;
                     coef = MMDAgent_str2float(&(p3[1]));
                  } else {
                     thres = false;
                     coef = MMDAgent_str2float(p3);
                  }
               }
               PMDFaceInterface *f = getARKitMorph(p1);
               if (f) {
                  f->add(pmd, p2, coef, thres);
               } else {
                  f = new PMDFaceInterface(pmd, p2, coef, thres);
                  if (f->isValid())
                     m_arKitMorph->add(p1, MMDAgent_strlen(p1), (void *)f);
                  else
                     delete f;
               }
            }
         }
      }
      delete zf;
   }
   return true;
}

/* ShapeMap::getEyeRotationCoef: get eye rotation coef */
float ShapeMap::getEyeRotationCoef()
{
   return m_eyeRotationCoef;
}

/* ShapeMap::getLipMorph: get lip morph */
PMDFaceInterface *ShapeMap::getLipMorph(const char *lipEntryName)
{
   char **data;

   if (lipEntryName == NULL)
      return NULL;
   if (m_lipMorph == NULL)
      return NULL;
   if (m_lipMorph->search(lipEntryName, MMDAgent_strlen(lipEntryName), (void **)&data) == false)
      return NULL;
   return (PMDFaceInterface *)data;
}

/* ShapeMap::getTrackBone: get bone for head tracking */
PMDBone *ShapeMap::getTrackBone(const char *trackingBoneEntryName)
{
   char **data;

   if (trackingBoneEntryName == NULL)
      return NULL;
   if (m_trackBone == NULL)
      return NULL;
   if (m_trackBone->search(trackingBoneEntryName, MMDAgent_strlen(trackingBoneEntryName), (void **)&data) == false)
      return NULL;
   return (PMDBone *)data;
}

/* ShapeMap::etAUMorph: get morph for Action Unit based face tracking */
PMDFaceInterface *ShapeMap::getAUMorph(const char *auName)
{
   char **data;

   if (auName == NULL)
      return NULL;
   if (m_auMorph == NULL)
      return NULL;
   if (m_auMorph->search(auName, MMDAgent_strlen(auName), (void **)&data) == false)
      return NULL;
   return (PMDFaceInterface *)data;
}

/* ShapeMap::getARKitMorph: get morph for ARKit based face tracking */
PMDFaceInterface *ShapeMap::getARKitMorph(const char *shapeName)
{
   char **data;

   if (shapeName == NULL)
      return NULL;
   if (m_arKitMorph == NULL)
      return NULL;
   if (m_arKitMorph->search(shapeName, MMDAgent_strlen(shapeName), (void **)&data) == false)
      return NULL;
   return (PMDFaceInterface *)data;
}

/* ShapeMap::getActionMotionFileName: get file name for the specified dialogue action */
const char *ShapeMap::getActionMotionFileName(const char *actionName)
{
   char **data;

   if (actionName == NULL)
      return NULL;
   if (m_actionMotion == NULL)
      return NULL;
   if (m_actionMotion->search(actionName, MMDAgent_strlen(actionName), (void **)&data) == false)
      return NULL;
   return (const char *)data;
}

/* ShapeMap::getExBone: get bone for extra bone control */
PMDBone *ShapeMap::getExBone(const char *boneEntryName)
{
   char **data;

   if (boneEntryName == NULL)
      return NULL;
   if (m_exBone == NULL)
      return NULL;
   if (m_exBone->search(boneEntryName, MMDAgent_strlen(boneEntryName), (void **)&data) == false)
      return NULL;
   return (PMDBone *)data;
}

/* ShapeMap::getExMorph: get morph for extra morph control */
PMDFaceInterface *ShapeMap::getExMorph(const char *morphEntryName)
{
   char **data;

   if (morphEntryName == NULL)
      return NULL;
   if (m_exMorph == NULL)
      return NULL;
   if (m_exMorph->search(morphEntryName, MMDAgent_strlen(morphEntryName), (void **)&data) == false)
      return NULL;
   return (PMDFaceInterface *)data;
}

/* ShapeMap::doMorphTune: apply morph weight post-tune */
void ShapeMap::doMorphTune()
{
   int i;
   float rate_left, r;
   PMDFaceInterface *f;

   for (i = 0; i < m_morphTuneNum; i++) {
      rate_left = 1.0f;
      for (int j = 0; j < m_morphTuneLen[i]; j++) {
         f = m_morphTuneSet[i][j];
         r = f->getAssignedWeight();
         f->forceAssignedWeight(r * rate_left);
         rate_left *= 1.0f - r;
         if (rate_left < 0.0f)
            rate_left = 0.0f;
      }
   }
}

/* ShapeMap::getLipIgnoreList: get lip ignore list */
const char *ShapeMap::getLipIgnoreList()
{
   return m_lipIgnoreList;
}
