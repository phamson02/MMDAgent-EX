/*
  Copyright 2022-2023  Nagoya Institute of Technology

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/* RenderOffScreen: off-screen render class with shader */
class RenderOffScreen
{
private:
   GLuint m_framebuffer;          /* frame buffer for off-screen rendering */
   GLuint m_renderbuffer;         /* rendering buffer for framebuffer */
   GLuint m_textureColorbuffer;   /* texture buffer for framebuffer (multisample) */
   GLuint m_resolveFramebuffer;   /* frame buffer for resolving multisample texture */
   GLuint m_resolvedTexture;      /* resolved texture */
   GLuint m_processFramebuffer;   /* second frame buffer for second pass rendering */
   GLuint m_processTextureColorbuffer;  /* second texture buffer for second pass rendering */
   GLuint m_quadVBO;              /* VBO for drawing resolve texture on screen */
   GLuint m_quadVAO;              /* VAO for drawing resolve texture on screen */
   bool m_buffersActive;          /* true when buffers are assigned */
   bool m_buffersRequireInit;     /* true when buffers are just assigned and needs initialization */

   GLuint m_shaderProgram;   /* shader program id */
   bool m_shaderLoaded;      /* true when shader has been loaded */
   GLchar m_infoLog[512];    /* shader info log string */

   int m_width;            /* screen width */
   int m_height;           /* screen height */

   bool m_enabled;         /* true when off-screen rendering is enabled now */
   float m_intensity;      /* intensity of effect */
   float m_scalingFactor;  /* scale of effect */

   /* initialize: initialzie Render */
   void initialize();

   /* clear: free Render */
   void clear();

   /* loadShader: load shader programs */
   bool loadShader();

   /* initBuffers: initialize buffers */
   bool initBuffers();

   /* clearBuffers: clear buffers */
   void clearBuffers();

public:

   /* RenderOffScreen: constructor */
   RenderOffScreen();

   /* ~RenderOffScreen: destructor */
   ~RenderOffScreen();

   /* getInfoLog: get info log */
   const char *getInfoLog();

   /* setup: initialize and setup */
   bool setup(int width, int height);

   /* changeScreenSize: change screen size */
   void changeScreenSize(int width, int height);

   /* start: start off-screen rendering */
   void start();

   /* finish: finish off-screen rendering */
   void finish();

   /* getFrameBufferId: get frame buffer id */
   GLuint getFrameBufferId();

   /* getFrameBufferRequireInit: return flag if the current frame buffer requires surface initialization */
   bool getFrameBufferRequireInit();

   /* setFrameBufferRequireInit: set flag if the current frame buffer requires surface initialization */
   void setFrameBufferRequireInit(bool flag);

   /* getIntensity: get intensity */
   float getIntensity();

   /* setIntensity: set intensity */
   void setIntensity(float f);

   /* toggleIntensity: toggle between a set of intensity values */
   void toggleIntensity();

   /* getScaling: get scaling */
   float getScaling();

   /* setScaling: set scaling */
   void setScaling(float f);

   /* toggleScaling: toggle between a set of scaling values */
   void toggleScaling();

};
