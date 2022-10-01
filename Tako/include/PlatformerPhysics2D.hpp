#pragma once
#include "Math.hpp"
#include <vector>

namespace tako::Jam::PlatformerPhysics2D
{
	struct Rect
	{
		float x;
		float y;
		float w;
		float h;

		constexpr Rect() : x(0), y(0), w(0), h(0) {}
		constexpr Rect(float x, float y , float w, float h) : x(x), y(y), w(w), h(h) {}
		constexpr Rect(tako::Vector2 pos, tako::Vector2 size) : x(pos.x), y(pos.y), w(size.x), h(size.y) {}

		float Left()
		{
			return x - w / 2;
		}

		float Right()
		{
			return x + w / 2;
		}

		float Top()
		{
			return y + h / 2;
		}

		float Bottom()
		{
			return y - h / 2;
		}

		static bool Overlap(Rect a, Rect b)
		{
			return OverlapX(a, b) && OverlapY(a, b);
		}

		static bool OverlapX(Rect a, Rect b)
		{
			return std::abs(a.x - b.x) < a.w / 2 + b.w / 2;
		}

		static bool OverlapY(Rect a, Rect b)
		{
			return std::abs(a.y - b.y) < a.h / 2 + b.h / 2;
		}

		static float OverlapDiffX(Rect a, Rect b)
		{
			return a.w / 2 + b.w / 2 - std::abs(a.x - b.x);
		}

		static float OverlapDiffY(Rect a, Rect b)
		{
			return  a.h / 2 + b.h / 2 - std::abs(a.y - b.y);
		}

	};

	struct Node
	{
		Vector2& position;
		Vector2& velocity;
		Rect bounds;
		void* userData;
		Vector2 movement;

		Rect CalcRec()
		{
			return {position.x + bounds.x, position.y + bounds.y, bounds.w, bounds.h};
		}
	};

	struct TileCollisionMap
	{
		std::vector<int>& tiles;
		Vector2 tileSize;
		int width;
		int height;
	};

	static void CalculateMovement(float timeStep, std::vector<Node>& nodes)
	{
		for (Node& node : nodes)
		{
			node.movement = node.velocity * timeStep;
		}
	}

	namespace
	{
		inline float& AccessVectorOffset(Vector2& vec, size_t axisOffset)
		{
			auto charPtr = ((char*) &vec) + axisOffset;
			return *((float*)charPtr);
		}

		template<typename ColCallback>
		void StepAxis(Node& node, std::vector<Node>& nodes, TileCollisionMap& tilemap, size_t axisOffset, ColCallback callback)
		{
			float& mov = AccessVectorOffset(node.movement, axisOffset);
			float& pos = AccessVectorOffset(node.position, axisOffset);
			float& vel = AccessVectorOffset(node.velocity, axisOffset);
			float step = mathf::clamp(mov, -1, 1);
			pos += step;
			auto nodeRec = node.CalcRec();
			Node* collidedWith = nullptr;
			float collidedWithOverlap = -1;
			for (Node& other : nodes)
			{
				if (&node == &other)
				{
					continue;
				}

				auto otherRec = other.CalcRec();
				if (Rect::Overlap(nodeRec, otherRec))
				{
					auto overlap = axisOffset == offsetof(Vector2, x) ? Rect::OverlapDiffX(nodeRec, otherRec) : Rect::OverlapDiffY(nodeRec, otherRec);
					if (overlap > collidedWithOverlap)
					{
						collidedWith = &other;
						collidedWithOverlap = overlap;
					}
				}
			}

			int gridX = nodeRec.x / tilemap.tileSize.x;
			int gridY = tilemap.height - nodeRec.y / tilemap.tileSize.y;
			for (int x = -1; x <= 1; x++)
			{
				auto xg = gridX + x;
				if (xg < 0 || xg >= tilemap.width) continue;
				for (int y = -1; y <= 1; y++)
				{
					auto yg = gridY + y;
					if (yg < 0 || yg >= tilemap.height) continue;
					if (tilemap.tiles[xg + yg * tilemap.width])
					{
						auto gridRec = Rect(xg * tilemap.tileSize.x + tilemap.tileSize.x / 2, tilemap.height * tilemap.tileSize.y - yg * tilemap.tileSize.y - tilemap.tileSize.y / 2, tilemap.tileSize.x, tilemap.tileSize.y);
						if (Rect::Overlap(nodeRec, gridRec))
						{
							auto overlap = axisOffset == offsetof(Vector2, x) ? Rect::OverlapDiffX(nodeRec, gridRec) : Rect::OverlapDiffY(nodeRec, gridRec);
							if (overlap > collidedWithOverlap)
							{
								collidedWith = nullptr;
								collidedWithOverlap = overlap;
							}
						}
					}
				}
			}
			if (collidedWithOverlap > 0)
			{
				pos -= collidedWithOverlap * mathf::sign(step);
				step = 0;
				mov = 0;
				vel = 0;
				if (collidedWith)
				{
					callback(node, *collidedWith);
				}
			}
			if (mathf::abs(step) < 1)
			{
				mov = 0;
			}
			else
			{
				mov -= step;
			}
		}
	}

	template<typename ColCallback>
	void SimulatePhysics(std::vector<Node>& nodes, TileCollisionMap tilemap, ColCallback callback)
	{
		bool hadDelta = true;
		while (hadDelta)
		{
			hadDelta = false;

			//Horizontal
			for (Node& node : nodes)
			{
				if (node.movement.x == 0)
				{
					continue;
				}

				hadDelta = true;
				StepAxis(node, nodes, tilemap, offsetof(Vector2, x), callback);
			}

			//Vertical
			for (Node& node : nodes)
			{
				if (node.movement.y == 0)
				{
					continue;
				}

				hadDelta = true;
				StepAxis(node, nodes, tilemap, offsetof(Vector2, y), callback);
			}
		}
	}
}
