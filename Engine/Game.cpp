/****************************************************************************************** 
 *	Chili DirectX Framework Version 16.07.20											  *	
 *	Game.cpp																			  *
 *	Copyright 2016 PlanetChili.net <http://www.planetchili.net>							  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "MainWindow.h"
#include "Game.h"
#include <sstream>

Game::Game( MainWindow& wnd )
	:
	wnd( wnd ),
	gfx( wnd ),
	doc("Final Data Structure.csv", rapidcsv::LabelParams(0, -1))
{
	// place node 1 at the center of the screen
	const Vec2 center = Vec2(gfx.ScreenWidth / 2, gfx.ScreenHeight / 2);
	g.AddVertex(Node(1, center));
	// generate all the other nodes in a circle around node 1
	for (size_t i = 1; i < NumVertices; i++)
	{
		// the angle of the node from the center of the screen
		const float angle = 2 * 3.14f * i / (NumVertices - 1);
		// the position of the node on the screen
		const Vec2 pos = center + Vec2{ sin(angle), -cos(angle) } * 200.0f;
		// create a new node
		g.AddVertex(Node(i + 1, pos));
	}

	// 
	for (size_t i = 0, k = 0; i < doc.GetRowCount() && k < NumVertices; i++)
	{
		const size_t src = doc.GetCell<size_t>(SrcColIdx, i);
		const size_t numNeighbours = doc.GetCell<size_t>(NeighbourCountColIdx, i);
		
		if (g.GetAdjList_idx(src - 1).empty())
		{
			k++;
			for (size_t j = 0; j < numNeighbours; j++)
			{
				const size_t neighbour_j = doc.GetCell<size_t>(firstNeighbourColIdx + 3 * j, i);
				g.AddEdge_idx(src - 1, neighbour_j - 1);
			}
		}
	}

	wnd.kbd.EnableAutorepeat();
}

void Game::Go()
{
	gfx.BeginFrame();	
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModel()
{
	while (!wnd.kbd.KeyIsEmpty())
	{
		auto e = wnd.kbd.ReadKey();
		if (e.IsPress())
		{
			ProcessKey(e.GetCode());

			src = std::max(size_t(1), std::min(size_t(NumVertices), src));
			dst = std::max(size_t(1), std::min(size_t(NumVertices), dst));

			if (useBFS)
				path = g.BFS(Node(src), Node(dst));
			else
				path = g.DFS(Node(src), Node(dst));

			std::ostringstream oss;
			oss << (useBFS ? "BFS" : "DFS")
				<< " from " << src
				<< " to " << dst;
			msg = oss.str();
		}
	}
}

void Game::ProcessKey(unsigned char key)
{
	switch (key)
	{
	case VK_RIGHT:
		src++;
		break;
	case VK_LEFT:
		src--;
		break;
	case VK_UP:
		dst++;
		break;
	case VK_DOWN:
		dst--;
		break;
	case VK_SPACE:
		useBFS = !useBFS;
		break;
	}
}

void Game::ComposeFrame()
{
	const auto& nodes = g.GetVertices();

	// draw all the edges b/w the nodes
	for (size_t i = 0; i < NumVertices; i++)
	{
		// get the adj list for vertex i
		const auto& adj = g.GetAdjList_idx(i);
		// draw each edge in the adj list
		for (auto& edge : adj)
		{
			gfx.DrawLine(
				nodes[edge.src_idx].GetPos(),  // from src node
				nodes[edge.dst_idx].GetPos(),  // to dst idx
				Colors::Gray
			);
		}
	}

	// draw the nodes at their positions
	for (auto& node : nodes)
	{
		node.Draw(gfx, Colors::Cyan);
	}

	// draw the current highlighted path
	for (int i = 0; i < (int)path.size() - 1; i++)
	{
		// draw lines between the current vtx and the next
		gfx.DrawLine(path[i].GetPos(), path[i + 1].GetPos(), Colors::Red);
	}

	// redraw vertices that appear in the path red
	for (auto& v : path)
	{
		v.Draw(gfx, Colors::Red);
	}

	font.DrawText(msg, Vei2(20, gfx.ScreenHeight - 40), Colors::Gray, gfx);
}
