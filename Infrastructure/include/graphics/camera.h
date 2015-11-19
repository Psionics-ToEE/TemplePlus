
#pragma once

#include <DirectXMath.h>
using namespace DirectX;

/**
 * Defines the projection and position of the camera.
 */
class WorldCamera {
public:
		
	const XMFLOAT4X4& GetViewProj() {
		if (mDirty) {
			Update();
			mDirty = false;
		}
		return mViewProjection;
	}

	const XMFLOAT4X4& GetProj() {
		if (mDirty) {
			Update();
			mDirty = false;
		}
		return mProjection;
	}

	void SetTranslation(float x, float y) {
		mXTranslation = x;
		mYTranslation = y;
		mDirty = true;
	}
	
	XMFLOAT2 GetTranslation() const {
		return{ mXTranslation, mYTranslation };
	}

	void SetScreenWidth(float width, float height) {
		mScreenWidth = width;
		mScreenHeight = height;
		mDirty = true;
	}

	void SetScale(float scale) {
		mScale = scale;
		mDirty = true;
	}

	void SetIdentityTransform(bool enable) {
		mIdentityTransform = enable;
		mDirty = true;
	}

	/**
	 * Transforms a world coordinate into the local coordinate 
	 * space of the screen (in pixels).
	 */
	XMFLOAT2 WorldToScreen(XMFLOAT3 worldPos);

	/**
	 * Transforms a screen coordinate relative to the upper left
	 * corner of the screen into a world position with y = 0.
	 */
	XMFLOAT3 ScreenToWorld(float x, float y);

	void CenterOn(float x, float y, float z);

	const XMFLOAT4X4 &GetUiProjection() {
		if (mDirty) {
			Update();
			mDirty = false;
		}
		return mUiProjection;
	}

private:
	float mXTranslation = 0.0f;
	float mYTranslation = 0.0f;
	float mScale = 1.0f;
	float mScreenWidth;
	float mScreenHeight;
	bool mDirty = true;
	bool mIdentityTransform = false;

	// This is roughly 64 * PIXEL_PER_TILE (inches per tile to be precise)
	static constexpr float zNear = -1814.2098860142813876543089825124f;
	static constexpr float zFar = 1814.2098860142813876543089825124f;
	
	XMFLOAT4X4 mProjection;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mViewProjection;
	XMFLOAT4X4 mInvViewProjection;
	XMFLOAT4X4 mUiProjection;

	void Update();
};