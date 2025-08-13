module;
#include <manifold/manifold.h>
#include "Reflection.hpp"
export module Tako.CSG;

import Tako.Reflection;

import Tako.Math;

namespace tako::CSG
{
	export enum class CSGOperation
	{
		Union,
		Subtraction,
		Intersection
	};
}
REFLECT_ENUM(tako::CSG::CSGOperation, Union, Subtraction, Intersection)

namespace tako::CSG
{
	export struct CSGVertex
	{
		Vector3 position;
		Vector3 normal;
		Vector2 uv;
	};

	manifold::OpType Convert(CSGOperation op)
	{
		switch (op)
		{
			case CSGOperation::Union: return manifold::OpType::Add;
			case CSGOperation::Subtraction: return manifold::OpType::Subtract;
			case CSGOperation::Intersection: return manifold::OpType::Intersect;
		}
		ASSERT(false, "Invalid CSGOperation");
	}

	export struct CSGMeshOutput
	{
		std::vector<CSGVertex> vertices;
		std::vector<U16> indices;
	};

	export class CSGNode
	{
	public:
		virtual manifold::Manifold GetManifold() const = 0; //TODO: Hide manifold for users
	};

	export struct CSGBrush : public CSGNode
	{
		REFLECT_POLY(CSGBrush)
	};

	export struct CSGTransform
	{
		Vector3 position = {0, 0, 0};
		Vector3 rotation = {0, 0, 0};
		Vector3 scale = {1, 1, 1};

		REFLECT(CSGTransform, position, rotation, scale)
	};

    export struct CSGCombinerNode
    {
        std::unique_ptr<CSGBrush> node;
        CSGOperation operation = CSGOperation::Union;
		CSGTransform transform;

		manifold::Manifold GetManifold() const
		{
			auto& translation = transform.position;
			auto& rotation = transform.rotation;
			auto& scale = transform.scale;
			return node->GetManifold()
				.Translate({translation.x, translation.y, translation.z})
				.Rotate(rotation.x, rotation.y, rotation.z)
				.Scale({scale.x, scale.y, scale.z});
		}

		REFLECT(CSGCombinerNode, node, operation, transform)
    };

	export struct CSGCombiner : public CSGBrush
	{
		std::vector<CSGCombinerNode> nodes;

        /*
		template<typename... Nodes>
		CSGCombiner(Nodes&&... argNodes)
		{
			(nodes.emplace_back(std::make_unique<std::decay_t<Nodes>>(std::forward<Nodes>(argNodes))), ...);
		}
        */

		template<typename Node>
			requires std::derived_from<std::decay_t<Node>, CSGNode>
		CSGCombiner& Add(Node&& node, CSGOperation operation)
		{
			nodes.emplace_back(std::make_unique<CSGNode>(std::move(node)), operation);
			return *this;
		}

		manifold::Manifold GetManifold() const override
		{
			manifold::Manifold res;
			for (auto& node : nodes)
			{
				res = res.Boolean(node.GetManifold(), Convert(node.operation));
			}
			return res;
		}

		REFLECT_CHILD(CSGCombiner, nodes)
	};

	export struct BoxBrush : public CSGBrush
	{
		Vector3 extents;

		BoxBrush() : extents(0, 0, 0) {}
		BoxBrush(Vector3 extents) : extents(extents) {}
		BoxBrush(float x, float y, float z) : extents(x, y, z) {}

		manifold::Manifold GetManifold() const override
		{
			return manifold::Manifold::Cube({ extents.x, extents.y, extents.z }, true);
		}

		REFLECT_CHILD(BoxBrush, extents)
	};

	export struct SphereBrush : public CSGBrush
	{
		float radius;
		int resolution;

		SphereBrush() : radius(0), resolution(32) {}

		manifold::Manifold GetManifold() const override
		{
			return manifold::Manifold::Sphere(radius, resolution);
		}

		REFLECT_CHILD(SphereBrush, radius, resolution)
	};

	export struct CylinderBrush : public CSGBrush
	{
		float height;
		float radius;
		int resolution;

		CylinderBrush() : height(0), radius(0), resolution(32) {}
		CylinderBrush(float height, float radius, int resolution = 32) : height(height), radius(radius), resolution(resolution) {}

		manifold::Manifold GetManifold() const override
		{
			return manifold::Manifold::Cylinder(height, radius, -1, resolution, true)
				.Rotate(90, 0, 0);
		}

		REFLECT_CHILD(CylinderBrush, height, radius, resolution)
	};

	export class CSGResult : public CSGNode
	{
	public:

		CSGResult(manifold::Manifold&& manifold) : manifold(std::move(manifold))
		{
		}

		manifold::Manifold GetManifold() const override
		{
			return manifold;
		}
	private:
		manifold::Manifold manifold;
	};

	export CSGResult Boolean(const CSGNode& a, const CSGNode& b, CSGOperation op)
	{
		auto result = a.GetManifold().Boolean(b.GetManifold(), Convert(op));
		return CSGResult(std::move(result));
	}

	export CSGResult Union(const CSGNode& a, const CSGNode& b)
	{
		return CSGResult(a.GetManifold() + b.GetManifold());
	}

	export CSGResult Subtraction(const CSGNode& a, const CSGNode& b)
	{
		return CSGResult(a.GetManifold() - b.GetManifold());
	}

	export CSGResult Intersection(const CSGNode& a, const CSGNode& b)
	{
		return CSGResult(a.GetManifold() ^ b.GetManifold());
	}

	export CSGResult Translation(const CSGNode& node, const Vector3& translation)
	{
		return CSGResult(node.GetManifold().Translate({translation.x, translation.y, translation.z}));
	}

	export CSGMeshOutput Triangulate(const CSGNode& node)
	{
		auto manifold = node.GetManifold().CalculateNormals(0);
		auto mesh = manifold.GetMeshGL();
		CSGMeshOutput out;
		for (int i = 0; i < mesh.vertProperties.size(); i += mesh.numProp)
		{
			CSGVertex vert;
			vert.position = { mesh.vertProperties[i], mesh.vertProperties[i + 1], mesh.vertProperties[i + 2] };
			vert.normal = { mesh.vertProperties[i + 3], mesh.vertProperties[i + 4], mesh.vertProperties[i + 5] };
			out.vertices.emplace_back(vert);
		}
		for (int i = 0; i < mesh.triVerts.size(); i++)
		{
			out.indices.push_back(mesh.triVerts[i]);
		}
		return out;
	}
}
