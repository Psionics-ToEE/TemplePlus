#include "stdafx.h"

#include <graphics/shaperenderer2d.h>
#include <graphics/shaperenderer3d.h>
#include <graphics/textures.h>
#include <graphics/device.h>

#include <util/fixes.h>

#include <ui/ui.h>
#include <ui/ui_assets.h>
#include "tig/tig_startup.h"
#include "tig/tig_texture.h"

#include "render_hooks.h"

static RenderHooks fix;

bool RenderHooks::specialZ = false;

void RenderHooks::apply() {
	replaceFunction(0x101DA2D0, ShaderRender3d);
	replaceFunction(0x101D8CE0, ShaderRender2d);
	replaceFunction(0x101D9300, TextureRender2d);
	replaceFunction(0x101E8460, RenderImgFile);
	replaceFunction(0x101D90B0, RenderTexturedQuad);
	replaceFunction(0x101D8B70, RenderRectIndirect);
	replaceFunction(0x101D61F0, RenderLine3d);
	replaceFunction(0x1018B200, RenderRectInt);

	replaceFunction(0x10107050, RenderDisc3d);
	replaceFunction(0x101D60B0, SetDrawCircleZMode);
	replaceFunction(0x10106B70, DrawCircle3d);

	redirectCall(0x1013C556, DrawRadialMenuSegment1); // 17 segments
	redirectCall(0x1013D4B7, DrawRadialMenuSegment2); // 99 segments
	
}

int RenderHooks::ShaderRender3d(int vertexCount, XMFLOAT4* pos, XMFLOAT4* normals, XMCOLOR* diffuse, XMFLOAT2* uv, int primCount, uint16_t* indices, int shaderId) {
	/*
	// Remove special material marker in the upper byte and only 
	// use the actual shader registration id
	shaderId &= 0x00FFFFFF;

	// Shader has to have been registered earlier
	auto mdfFactory = static_cast<MdfMaterialFactory*>(gfx::gMdfMaterialFactory);
	auto material = mdfFactory->GetById(shaderId);

	if (!material->IsValid()) {
		logger->error("Legacy shader with id {} wasn't found.", shaderId);
		return 17;
	}

	auto mdfMaterial = static_cast<MdfRenderMaterial*>(material.get());

	renderStates->SetProjectionMatrix(renderStates->Get3dProjectionMatrix());
	renderStates->SetZEnable(true);
	renderStates->SetZWriteEnable(true);

	MdfRenderer renderer(*graphics);
	renderer.Render(mdfMaterial,
	                vertexCount,
	                pos,
	                normals,
	                diffuse,
	                uv,
	                primCount,
	                indices);

	renderStates->SetZEnable(false);
	renderStates->SetZWriteEnable(false);
	*/
	return 0;
}

int RenderHooks::ShaderRender2d(const Render2dArgs* args) {

	if (!args->shaderId) {
		return 17;
	}

	// Resolve the MDF
	auto material(tig->GetMdfFactory().GetById(args->shaderId));

	if (!material) {
		return 17; // Unknown shader id
	}
	
	std::array<gfx::Vertex2d, 4> corners;

	auto left = (float)args->destRect->x;
	auto top = (float)args->destRect->y;
	auto right = (float)args->destRect->x + args->destRect->width;
	auto bottom = (float) args->destRect->y + args->destRect->height;
	
	if (!args->vertexColors) {
		corners[0].diffuse = { 1, 1, 1, 1 };
		corners[1].diffuse = { 1, 1, 1, 1 };
		corners[2].diffuse = { 1, 1, 1, 1 };
		corners[3].diffuse = { 1, 1, 1, 1 };
	} else {
		corners[0].diffuse = args->vertexColors[0];
		corners[1].diffuse = args->vertexColors[1];
		corners[2].diffuse = args->vertexColors[2];
		corners[3].diffuse = args->vertexColors[3];
	}

	corners[0].pos = { left, top, 0, 1 };
	
	corners[0].uv = { 0, 0 };
	corners[1].pos = { right, top, 0, 1 };
	corners[1].uv = { 1, 0 };
	corners[2].pos = { right, bottom, 0, 1 };
	corners[2].uv = { 1, 1 };
	corners[3].pos = { left, bottom, 0, 1 };
	corners[3].uv = { 0, 1 };

	tig->GetShapeRenderer2d().DrawRectangle(corners, material);

	/*
	if (!args->shaderId)
		return 17;

	// Shader has to have been registered earlier
	auto mdfFactory = static_cast<MdfMaterialFactory*>(gfx::gMdfMaterialFactory);
	auto material = mdfFactory->GetById(args->shaderId);

	if (!material || !material->IsValid()) {
		logger->error("Legacy shader with id {} wasn't found.", args->shaderId);
		return 17;
	}

	auto texture = material->GetPrimaryTexture();
	if (!texture->IsValid()) {
		logger->error("Legacy shader with id {} doesn't have a primary texture.");
		return 17;
	}

	auto textureSize = texture->GetSize();
	auto contentRect = texture->GetContentRect();

	static std::array<uint16_t, 6> indices{
		0, 1, 2, 2, 3, 0
	};

	std::array<XMFLOAT2, 4> uv;
	std::array<XMFLOAT4, 4> vertices;
	std::array<XMFLOAT4, 4> normals;

	auto vertexZ = 0.5f;
	if (args->flags & Render2dArgs::FLAG_VERTEXZ) {
		vertexZ = args->vertexZ;
	}

	XMCOLOR* diffuse = nullptr;
	if (args->flags & Render2dArgs::FLAG_VERTEXCOLORS) {
		diffuse = args->vertexColors;
	}

	auto left = args->srcRect->x;
	auto right = args->srcRect->x + args->srcRect->width;
	auto top = args->srcRect->y;
	auto bottom = args->srcRect->y + args->srcRect->height;
	if (args->flags & 0x10) {
		left = textureSize.width - left - 1;
		right = textureSize.width - right - 1;
	}
	if (args->flags & 0x20) {
		top = textureSize.height - top - 1;
		bottom = textureSize.height - bottom - 1;
	}

	for (auto& normal : normals) {
		normal.x = 0;
		normal.y = 1;
		normal.z = 0;
	}

	auto destRect = args->destRect;
	auto widthFactor = 1.0f / (config.renderWidth / 2);
	auto heightFactor = 1.0f / (config.renderHeight / 2);

	vertices[0].x = destRect->x * widthFactor - 1.0f;
	vertices[0].y = 1.0f - destRect->y * heightFactor;
	vertices[0].z = vertexZ;
	uv[0].x = (left + 0.5f) / contentRect.width;
	uv[0].y = (top + 0.5f) / contentRect.height;

	vertices[1].x = (destRect->x + destRect->width) * widthFactor - 1.0f;
	vertices[1].y = 1.0f - destRect->y * heightFactor;
	vertices[1].z = vertexZ;
	uv[1].x = (right + 0.5f) / contentRect.width;
	uv[1].y = uv[0].y;

	vertices[2].x = (destRect->x + destRect->width) * widthFactor - 1.0f;
	vertices[2].y = 1.0f - (destRect->height + destRect->y) * heightFactor;
	vertices[2].z = vertexZ;
	uv[2].x = uv[1].x;
	uv[2].y = (bottom + 0.5f) / contentRect.height;

	vertices[3].x = destRect->x * widthFactor - 1.0f;
	vertices[3].y = 1.0f - (destRect->height + destRect->y) * heightFactor;
	vertices[3].z = vertexZ;
	uv[3].x = uv[0].x;
	uv[3].y = uv[2].y;

	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixIdentity());
	renderStates->SetProjectionMatrix(projMatrix);

	auto mdfMaterial = static_cast<MdfRenderMaterial*>(material.get());
	MdfRenderer renderer(*graphics);
	renderer.Render(mdfMaterial, 4, &vertices[0], &normals[0], &diffuse[0], &uv[0], 2, &indices[0]);
	*/
	return 0;
}

int RenderHooks::TextureRender2d(const Render2dArgs* args) {

	auto &shapeRenderer = tig->GetShapeRenderer2d();
	auto &textures = tig->GetRenderingDevice().GetTextures();

	float texwidth;
	float texheight;
	float srcX;
	float srcY;
	float srcWidth;
	float srcHeight;
	std::array<gfx::Vertex2d, 4> vertices;

	// The townmap UI uses floating point coordinates for the srcrect
	// for whatever reason. They are passed in place of the integer coordinates
	// And need to be reinterpreted
	if (args->flags & Render2dArgs::FLAG_FLOATSRCRECT) {
		srcX = *(float *)&args->srcRect->x;
		srcY = *(float *)&args->srcRect->y;
		srcWidth = *(float *)&args->srcRect->width;
		srcHeight = *(float *)&args->srcRect->height;
	} else {
		srcX = (float)args->srcRect->x;
		srcY = (float)args->srcRect->y;
		srcWidth = (float)args->srcRect->width;
		srcHeight = (float)args->srcRect->height;
	}

	// Has a special vertex z value been set? Otherwise we render all UI
	// on the same level
	auto vertexZ = 0.5f;
	if (args->flags & Render2dArgs::FLAG_VERTEXZ) {
		vertexZ = args->vertexZ;
	}

	// Inherit vertex colors from the caller
	if (args->flags & Render2dArgs::FLAG_VERTEXCOLORS) {
		// Previously, ToEE tried to compute some gradient stuff here
		// which we removed because it was never actually utilized properly
		vertices[0].diffuse = args->vertexColors[0] | 0xFF000000;
		vertices[1].diffuse = args->vertexColors[1] | 0xFF000000;
		vertices[2].diffuse = args->vertexColors[2] | 0xFF000000;
		vertices[3].diffuse = args->vertexColors[3] | 0xFF000000;
	} else {
		vertices[0].diffuse = 0xFFFFFFFF;
		vertices[1].diffuse = 0xFFFFFFFF;
		vertices[2].diffuse = 0xFFFFFFFF;		
		vertices[3].diffuse = 0xFFFFFFFF;
	}

	// Only if this flag is set, is the alpha value of 
	// the vertex colors used
	if (args->flags & Render2dArgs::FLAG_VERTEXALPHA) {
		vertices[0].diffuse.c &= args->vertexColors[0] | 0xFFFFFF;
		vertices[1].diffuse.c &= args->vertexColors[1] | 0xFFFFFF;
		vertices[2].diffuse.c &= args->vertexColors[2] | 0xFFFFFF;
		vertices[3].diffuse.c &= args->vertexColors[3] | 0xFFFFFF;
	}

	// Load the associated texture
	gfx::Texture* deviceTexture = nullptr;
	if (args->flags & Render2dArgs::FLAG_BUFFERTEXTURE) {
		// This is a custom flag we introduced for TP
		auto texture = (gfx::Texture*) args->texBuffer;
		deviceTexture = texture;
		
		auto size = texture->GetSize();
		texwidth = (float)size.width;
		texheight = (float)size.height;
	} else if (!(args->flags & Render2dArgs::FLAG_BUFFER)) {
		if (args->textureId) {
			auto texture = textures.GetById(args->textureId);
			if (!texture || !texture->IsValid()) {
				return 17;
			}

			if (texture->GetResourceView() == nullptr) {
				return 17;
			}
			deviceTexture = texture.get();

			auto size = texture->GetSize();
			texwidth = (float) size.width;
			texheight = (float) size.height;
		} else {
			texwidth = (float) args->destRect->width;
			texheight = (float) args->destRect->height;
		}
	} else {
		throw TempleException("Unsupported operation mode for TextureRender2d");
	}

	auto contentRectLeft = srcX;
	auto contentRectTop = srcY;
	auto contentRectRight = srcX + srcWidth;
	auto contentRectBottom = srcY + srcHeight;

	// Create the UV coordinates to honor the contentRect based 
	// on the real texture size
	auto uvLeft = (contentRectLeft) / texwidth;
	auto uvRight = (contentRectRight) / texwidth;
	auto uvTop = (contentRectTop) / texheight;
	auto uvBottom = (contentRectBottom) / texheight;
	vertices[0].uv.x = uvLeft;
	vertices[0].uv.y = uvTop;
	vertices[1].uv.x = uvRight;
	vertices[1].uv.y = uvTop;
	vertices[2].uv.x = uvRight;
	vertices[2].uv.y = uvBottom;
	vertices[3].uv.x = uvLeft;
	vertices[3].uv.y = uvBottom;

	// Flip the U coordinates horizontally
	if (args->flags & Render2dArgs::FLAG_FLIPH) {
		// Top Left with Top Right
		std::swap(vertices[0].uv.x, vertices[1].uv.x);
		// Bottom Right with Bottom Left
		std::swap(vertices[2].uv.x, vertices[3].uv.x);
	}
	// Flip the V coordinates horizontally
	if (args->flags & Render2dArgs::FLAG_FLIPV) {
		// Top Left with Bottom Left
		std::swap(vertices[0].uv.y, vertices[3].uv.y);
		// Top Right with Bottom Right
		std::swap(vertices[1].uv.y, vertices[2].uv.y);
	}

	float destX = (float) args->destRect->x;
	float destY = (float) args->destRect->y;

	if (args->flags & Render2dArgs::FLAG_ROTATE) {
		// Rotation?
		auto cosRot = cosf(args->rotation);
		auto sinRot = sinf(args->rotation);
		auto destRect = args->destRect;
		vertices[0].pos.x = args->rotationX
			+ (destX - args->rotationX) * cosRot
			- (destY - args->rotationY) * sinRot;
		vertices[0].pos.y = args->rotationY
			+ (destY - args->rotationY) * cosRot
			+ (destX - args->rotationX) * sinRot;
		vertices[0].pos.z = vertexZ;

		vertices[1].pos.x = args->rotationX
			+ ((destX + destRect->width) - args->rotationX) * cosRot
			- (destY - args->rotationY) * sinRot;
		vertices[1].pos.y = args->rotationY
			+ ((destX + destRect->width) - args->rotationX) * sinRot
			+ (destY - args->rotationY) * cosRot;
		vertices[1].pos.z = vertexZ;

		vertices[2].pos.x = args->rotationX
			+ ((destX + destRect->width) - args->rotationX) * cosRot
			- ((destY + destRect->width) - args->rotationY) * sinRot;
		vertices[2].pos.y = args->rotationY
			+ ((destY + destRect->width) - args->rotationY) * cosRot
			+ (destX + destRect->width - args->rotationX) * sinRot;
		vertices[2].pos.z = vertexZ;

		vertices[3].pos.x = args->rotationX
			+ (destX - args->rotationX) * cosRot
			- ((destY + destRect->height) - args->rotationY) * sinRot;
		vertices[3].pos.y = args->rotationY
			+ ((destY + destRect->height) - args->rotationY) * cosRot
			+ (destX - args->rotationX) * sinRot;
		vertices[3].pos.z = vertexZ;
	} else {
		auto destRect = args->destRect;
		vertices[0].pos.x = destX;
		vertices[0].pos.y = destY;
		vertices[0].pos.z = vertexZ;
		vertices[1].pos.x = destX + destRect->width;
		vertices[1].pos.y = destY;
		vertices[1].pos.z = vertexZ;
		vertices[2].pos.x = destX + destRect->width;
		vertices[2].pos.y = destY + destRect->height;
		vertices[2].pos.z = vertexZ;
		vertices[3].pos.x = destX;
		vertices[3].pos.y = destY + destRect->height;
		vertices[3].pos.z = vertexZ;
	}
	vertices[0].pos.w = 1;
	vertices[1].pos.w = 1;
	vertices[2].pos.w = 1;
	vertices[3].pos.w = 1;
	
	gfx::Texture *maskTexture = nullptr;
	// We have a secondary texture
	if (args->flags & Render2dArgs::FLAG_MASK) {
		auto texture = textures.GetById(args->textureId2);
		if (!texture || !texture->IsValid() || !texture->GetResourceView()) {
			return 17;
		}
		maskTexture = texture.get();
	}

	// This is used by the portrait UI to mask the equipment slot background when
	// rendering an icon
	auto blending = ((args->flags & Render2dArgs::FLAG_DISABLEBLENDING) == 0);

	gfx::SamplerType2d samplerType = gfx::SamplerType2d::CLAMP;
	if ((args->flags & Render2dArgs::FLAG_WRAP) != 0) {
		samplerType = gfx::SamplerType2d::WRAP;
	}

	shapeRenderer.DrawRectangle(vertices, deviceTexture, maskTexture, samplerType, blending);
	
	return 0;
}

int RenderHooks::RenderTexturedQuad(XMFLOAT3* vertices, float* u, float* v, int textureId, XMCOLOR color) {

	auto texture(tig->GetRenderingDevice().GetTextures().GetById(textureId));

	if (!texture->IsValid()) {
		return 17;
	}
	
	std::array<gfx::Vertex2d, 4> corners;

	*(XMFLOAT3*)&corners[0].pos = vertices[0];
	*(XMFLOAT3*)&corners[1].pos = vertices[1];
	*(XMFLOAT3*)&corners[2].pos = vertices[2];
	*(XMFLOAT3*)&corners[3].pos = vertices[3];
	corners[0].pos.w = 1;
	corners[1].pos.w = 1;
	corners[2].pos.w = 1;
	corners[3].pos.w = 1;

	corners[0].diffuse = color;
	corners[1].diffuse = color;
	corners[2].diffuse = color;
	corners[3].diffuse = color;

	corners[0].uv = {u[0],v[0]};
	corners[1].uv = {u[1],v[1]};
	corners[2].uv = {u[2],v[2]};
	corners[3].uv = {u[3],v[3]};

	tig->GetShapeRenderer2d().DrawRectangle(corners, texture.get());
	return 0;

}

void RenderHooks::RenderImgFile(ImgFile* img, int x, int y) {

	auto &shapeRenderer = tig->GetShapeRenderer2d();
	auto &textures = tig->GetRenderingDevice().GetTextures();
	
	auto texId = &img->textureIds[0];
	auto curX = 0;

	for (auto tileX = 0; tileX < img->tilesX; ++tileX) {

		// The last column fills the remaining space which is less than 256 pixels
		int colWidth;
		if (curX + 256 <= img->width) {
			colWidth = 256;
		} else {
			colWidth = img->width - curX;
		}

		auto curY = img->height;

		for (auto tileY = 0; tileY < img->tilesY; ++tileY) {

			auto rowHeight = std::min(256, curY);
			curY -= rowHeight;

			auto &texture = textures.GetById(*texId++);
			
			auto destX = static_cast<float>(x + curX);
			auto destY = static_cast<float>(y + curY);
			auto destWidth = static_cast<float>(colWidth);
			auto destHeight = static_cast<float>(rowHeight);
			shapeRenderer.DrawRectangle(destX, destY, destWidth, destHeight, *texture);
		}

		curX += colWidth;

	}

}

void RenderHooks::RenderRect(float left, float top, float right, float bottom, XMCOLOR color) {

	auto &shapeRenderer = tig->GetShapeRenderer2d();

	shapeRenderer.DrawRectangle(left, top, right - left, bottom - top, color);

}

void RenderHooks::RenderRectInt(int left, int top, int width, int height, XMCOLOR color) {

	XMFLOAT2 topLeft{ (float) left, (float) top };
	XMFLOAT2 bottomRight{ topLeft.x + width, topLeft.y + height };

	RenderRectIndirect(&topLeft, &bottomRight, color);
}

void RenderHooks::RenderDisc3d(LocAndOffsets& loc, int shaderId, float rotation, float radius) {

	auto material(tig->GetMdfFactory().GetById(shaderId));
	if (!material) {
		return;
	}
		
	auto center(loc.ToInches3D());
	tig->GetShapeRenderer3d().DrawDisc(center, rotation, radius, material);

}

int RenderHooks::RenderRectIndirect(XMFLOAT2* topLeft, XMFLOAT2* bottomRight, XMCOLOR color) {
	auto &shapeRenderer = tig->GetShapeRenderer2d();

	// In Vanilla, pixel coverage for these lines was ~50% leading to a darker
	// color being drawn. This was inconsistent across drivers/fullscreen/windowed,
	// so this ends up being an approximation...
	color.r /= 2;
	color.g /= 2;
	color.b /= 2;

	// To compensate for Vanilla's misuse of D3D8 line rasterization, we have to expand
	// the actual rectangle by 1 in both dimensions.
	XMFLOAT2 extendedBottomRight = *bottomRight;
	extendedBottomRight.x++;
	extendedBottomRight.y++;

	shapeRenderer.DrawRectangleOutline(*topLeft, extendedBottomRight, color);

	return 0;
}

int RenderHooks::RenderLine3d(XMFLOAT3* from, XMFLOAT3* to, XMCOLOR color) {

	tig->GetShapeRenderer3d().DrawLine(
		*from,
		*to,
		color
	);

	return 0;

}

int RenderHooks::DrawRadialMenuSegment1(int x, int y,
	float angleCenter, float angleWidth,
	int innerRadius, int innerOffset,
	int outerRadius, int outerOffset,
	XMCOLOR color1, XMCOLOR color2) {

	tig->GetShapeRenderer2d()
		.DrawPieSegment(9, x, y, angleCenter, angleWidth, innerRadius, innerOffset,
			outerRadius, outerOffset, color1, color2);

	return 0;
}

int RenderHooks::DrawRadialMenuSegment2(int x, int y, 
	float angleCenter, float angleWidth, 
	int innerRadius, int innerOffset, 
	int outerRadius, int outerOffset, 
	XMCOLOR color1, XMCOLOR color2) {

	tig->GetShapeRenderer2d()
		.DrawPieSegment(50, x, y, angleCenter, angleWidth, innerRadius, innerOffset,
			outerRadius, outerOffset, color1, color2);

	return 0;
}

/*
	This function is called right before a draw_circle function
	with argument 3, then called again afterwards with argument 2.
	Instead of propagating that to all draws, we store it here
	for the next draw circle call.

	The use case for this is to draw circles that are occluded 
	by walls in a slightly less opaque color.
*/
void RenderHooks::SetDrawCircleZMode(int type) {
	if (type == 3) {
		specialZ = true; // D3DCMP_GREATEREQUAL
	} else if(type == 2) {
		specialZ = false;
	} else {
		throw TempleException("Unsupported depth type: {}", type);
	}
}

void RenderHooks::DrawCircle3d(LocAndOffsets center, 
	float negElevation, 
	XMCOLOR fillColor,
	XMCOLOR borderColor,
	float radius) {
		
	auto y = -(sinf(-0.77539754f) * negElevation);
	auto center3d(center.ToInches3D(y));

	tig->GetShapeRenderer3d().DrawFilledCircle(
		center3d, radius, borderColor, fillColor, specialZ
	);

}
