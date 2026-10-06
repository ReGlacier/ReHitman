# Glacier PF4 (Pathfinding 4)

This directory contains Glacier's pathfinding system. It provides tools for navigating a mesh-based environment, handling static and dynamic obstacles, and querying paths between locations.

The system has three main parts:

1. `ZInterface` defines the abstract interface for pathfinding queries, such as finding paths, mapping locations to the navigation mesh, and managing obstacles.
2. `ZData` is the primary implementation of `ZInterface`. It owns the navigation mesh data (graphs, nodes, vertices, links, and components) and manages dynamic obstacles.
3. `ZPath` represents a calculated path, containing a sequence of vertices and associated actions.

## Basic Usage

To use the pathfinding system, you typically interact with a `ZInterface` instance (often a `ZData` object). You can request a path between two `ZLocation` points using a `ZPathRequest`.

```cpp
#include <Glacier/PF4/ZData.h>
#include <Glacier/PF4/ZPath.h>
#include <Glacier/PF4/ZLocation.h>

// Assuming pPF4Data is a valid Glacier::PF4::ZInterface*
Glacier::PF4::ZLocation source(vPosA, graphIdx, componentIdx, true);
Glacier::PF4::ZLocation dest(vPosB, graphIdx, componentIdx, true);
Glacier::PF4::ZPath path(pPF4Data, 64); // Max 64 vertices

Glacier::PF4::ZPathRequest request;
request.m_Source = &source;
request.m_Dest = &dest;
request.m_Path = &path;

if (pPF4Data->FindPath(&request))
{
    // Path found. Iterate through path.m_Vertices or use GetPosition()
    ZVector3 vPosition;
    path.GetPosition(0, vPosition);
}
```

## Core Components

### ZInterface
`ZInterface` is the abstract base class for all pathfinding providers. It defines the contract for:
- **Location Mapping**: `MapLocation()`, `PositionInside()`, `MapOntoComponent()`.
- **Pathfinding**: `FindPath()`, `FindPathLength()`.
- **Obstacle Management**: `AddObstacle()`, `RemoveObstacle()`, `MoveObstacle()`.
- **Spatial Queries**: `FindNodes()`, `FindComponents()`, `FindWalls()`.

### ZData
`ZData` is the central manager of the navigation system. It implements `ZInterface` and stores the following data structures:
- `ZGraph`: Sub-meshes of the navigable environment.
- `ZNode`: Navigation points within graphs.
- `ZVertex`: Geometric vertices used for path smoothing/representation.
- `ZLink`: Connections between nodes, including costs and associated actions (e.g., doors).
- `ZComponent`: Groups of nodes and vertices that form logical sections of the mesh.

`ZData` also manages a list of `ZDynamicObstacle` objects, which are integrated into the pathfinding queries to allow for real-time environment changes.

### ZPath
`ZPath` is the result of a pathfinding query. It stores a series of vertices (`ZDataRef` or custom vertices) that define the route. It can also store "actions" at specific points in the path, such as instructions to interact with a gate or door.

### ZLocation
`ZLocation` represents a point in the navigation world, combining a 3D position (`ZVector3`) with the specific `ZGraph` and `ZComponent` context.

### ZMetaNode
`ZMetaNode` represents an entity (such as an AI agent or player) that is registered within the pathfinding system to track its location and interact with the mesh.

## Data Format (Micro-specification)

The PF4 navigation data is stored as a single contiguous block of memory, structured as a series of packed arrays. The layout is determined by an `SPF4DataBlock` header at the beginning of the block.

### Layout Schema

1. `SPF4DataBlock` (0x1C): Header containing counts for all subsequent arrays.
2. `ZSplitTree` array (count: `m_iSplitTreeCount`, size: `0x10` per entry)
3. `ZSplitTree` array (count: `m_iHeightTreeCount`, size: `0x10` per entry)
4. `ZPlaneEquation` array (count: `m_iPlaneEquationCount`, size: `0xC` per entry)
5. `ZComponent` array (count: `m_iComponentCount`, size: `0x14` per entry)
6. `ZGraph` array (count: `m_iGraphCount`, size: `0x30` per entry)
7. `ZNode` array (count: `m_iNodeCount`, size: `0x14` per entry)
8. `ZVertex` array (count: `m_iVertexCount`, size: `0x14` per entry)
9. `ZLink` array (count: `m_iLinkCount`, size: `0x10` per entry)
10. `ZIndex` array (count: `m_iExitDistCount`, size: `0x2` per entry)
11. `ZCorner` array (count: `m_iCornerCount`, size: `0x4` per entry)
12. `ZSubNode` array (count: `m_iSubNodeCount`, size: `0x8` per entry)
13. `ZIndex` array (count: `m_iStaticObstacleIdCount`, size: `0x2` per entry)
14. `ZStaticObstacle` array (count: `m_iStaticObstacleCount`, size: `0x4` per entry)

### Indexing and Referencing

- **Nodes**: `ZGraph` defines its range within the global `ZNode` array using `m_iFirstNode`. `ZLink` entries use a local index `m_iNode` which is relative to the graph's `m_iFirstNode`.
- **Vertices**: `ZStaticObstacle` refers to a sequence of vertices in the global `ZVertex` array using `m_FirstVertex`.
- **Components**: `ZComponent` entries serve as containers for various sub-elements (corners, sub-nodes, obstacles, height trees) using relative indices.
- **ZDataRef**: A 4-byte handle used for lightweight referencing.
    - `bits 0..13`: Index/ID.
    - `bits 14..15`: Type (0: Node, 1: Vertex, 2: Link, 3: Custom Vertex).
    - `m_Pos`: For type 3, stores the 2D position (X/Z).
