#ifndef SOCOM_RENDER_H
#define SOCOM_RENDER_H

#include "game.h"

bool WorldToScreen(Vec3 world, Vec2* screen);

void spDrawLine(f32 x1, f32 y1, f32 x2, f32 y2, Vec4 color);
void wsDrawLine(Vec3 start, Vec3 end, Vec4 color);

// custom draw circle method
void Draw2DCircle(f32 x, f32 y, f32 radius, f32 thickness, Vec4 color);

// draws a bounding box around the input object in world space
void wsDrawBoundingBox(CNode* node, Vec3 color);

// draws a bounding box around the input object in screen space
void spDrawBoundingBox(CNode* node, Vec3 color);

// draws the skeleton of the input seal body in world space
void wsDrawSkeleton(CZSealBody* seal);

// draws the skeleton of the input seal body in screen space
void spDrawSkeleton(CZSealBody* seal);

#endif