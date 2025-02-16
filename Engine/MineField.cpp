#include "MineField.h"
#include <assert.h>
#include <algorithm>
#include <random>
#include "Vei2.h"
#include "SpriteCodex.h"

void MineField::Tile::SpawnMine () {
	assert (!has_mine);
	has_mine = true;
}

bool MineField::Tile::HasMine () const {
	return has_mine;
}

void MineField::Tile::Draw (const Vei2& screen_pos, Graphics& gfx, MineField::State game_state) const {
	if(game_state != MineField::State::lose) {
		switch(state) {
		case State::hidden:SpriteCodex::DrawTileButton (screen_pos, gfx); break;
		case State::flagged:SpriteCodex::DrawTileButton (screen_pos, gfx);
			SpriteCodex::DrawTileFlag (screen_pos, gfx); break;
		case State::revealed:
			if(!HasMine ()) {
				SpriteCodex::DrawTileNumber (screen_pos, gfx, n_neighbor_mines);
			} else {
				SpriteCodex::DrawTileBomb (screen_pos, gfx);
			} break;
		}
	} else {
		switch(state) {
		case State::hidden:
			if(has_mine) {
				SpriteCodex::DrawTileBomb (screen_pos, gfx); break;
			} else {
				SpriteCodex::DrawTileButton (screen_pos, gfx); break;
			}
		case State::flagged:
			if(has_mine) {
				SpriteCodex::DrawTileBomb (screen_pos, gfx);
				SpriteCodex::DrawTileFlag (screen_pos, gfx); break;
			} else {
				SpriteCodex::DrawTileBomb (screen_pos, gfx);
				SpriteCodex::DrawTileCross (screen_pos, gfx); break;
			}
		case State::revealed:
			if(!HasMine ()) {
				SpriteCodex::DrawTileNumber (screen_pos, gfx, n_neighbor_mines);
			} else {
				SpriteCodex::DrawTileBombRed (screen_pos, gfx);
				SpriteCodex::DrawTileBomb (screen_pos, gfx);
			} break;
		}
	}
}

void MineField::Tile::Reveal () {
	assert (state == State::hidden);
	state = State::revealed;
}

bool MineField::Tile::IsRevealed () const {
	return state == State::revealed;
}

void MineField::Tile::ToggleFlag () {
	assert (!IsRevealed ());
	if(state == State::hidden) {
		state = State::flagged;
	} else {
		state = State::hidden;
	}
}

bool MineField::Tile::IsFlagged () const {
	return state == State::flagged;
}

bool MineField::Tile::HasNoNeighborMines () const {
	return n_neighbor_mines == 0;
}

void MineField::Tile::SetNeighborMineCount (int mine_count) {
	assert (n_neighbor_mines == -1);
	n_neighbor_mines = mine_count;
}

MineField::MineField (const Vei2& center, int n_mines)
	:
	top_left (center - Vei2 (width * SpriteCodex::tileSize, height * SpriteCodex::tileSize) / 2)
{
	assert (n_mines > 0 && n_mines < width * height);
	std::random_device rd;
	std::mt19937 rng (rd ());
	std::uniform_int_distribution<int> xDist (0, width - 1);
	std::uniform_int_distribution<int> yDist (0, height - 1);

	for(int n_spawned = 0; n_spawned < n_mines; ++n_spawned) {
		Vei2 spawn_pos;
		do {
			spawn_pos = { xDist (rng), yDist (rng) };
		} while(TileAt(spawn_pos).HasMine());

		TileAt (spawn_pos).SpawnMine ();
	}

	for(Vei2 grid_pos = { 0, 0 }; grid_pos.y < height; ++grid_pos.y) {
		for(grid_pos.x = 0; grid_pos.x < width; ++grid_pos.x) {
			TileAt (grid_pos).SetNeighborMineCount (CountNeighborMines (grid_pos));
		}
	}
}

void MineField::Draw (Graphics& gfx) const { 
	gfx.DrawRect (GetRect ().GetExpanded(border_width), SpriteCodex::baseColor);
	for(Vei2 grid_pos = { 0, 0 }; grid_pos.y < height; ++grid_pos.y) {
		for(grid_pos.x = 0; grid_pos.x < width; ++grid_pos.x) {
			TileAt (grid_pos).Draw(top_left + grid_pos * SpriteCodex::tileSize, gfx, state);
		}
	}
}

RectI MineField::GetRect () const {
	return RectI (top_left, width * SpriteCodex::tileSize, height * SpriteCodex::tileSize);
}

void MineField::OnRevealClick (const Vei2& screen_pos) {
	if(state == State::play) {
		const Vei2 grid_pos = ScreenToGrid (screen_pos);
		assert (grid_pos.x >= 0 && grid_pos.x < width && grid_pos.y >= 0 && grid_pos.y < height);
		RevealTile (grid_pos);
		if(GameIsWon ()) {
			state = State::win;
		}
	}
}

void MineField::OnFlagClick (const Vei2& screen_pos) { 
	if(state == State::play) {
		const Vei2 grid_pos = ScreenToGrid (screen_pos);
		assert (grid_pos.x >= 0 && grid_pos.x < width && grid_pos.y >= 0 && grid_pos.y < height);
		Tile& tile = TileAt (grid_pos);
		if(!tile.IsRevealed ()) {
			tile.ToggleFlag ();
		}
	}
}

MineField::State MineField::GetState () const {
	return state;
}

void MineField::RevealTile (const Vei2& grid_pos) {
	Tile& tile = TileAt (grid_pos);
	if(!tile.IsRevealed () && !tile.IsFlagged ()) {
		tile.Reveal ();
		if(tile.HasMine ()) {
			state = State::lose;
		} else if(tile.HasNoNeighborMines ()) {
			const int x_start = std::max (0, grid_pos.x - 1);
			const int y_start = std::max (0, grid_pos.y - 1);
			const int x_end = std::min (width - 1, grid_pos.x + 1);
			const int y_end = std::min (height - 1, grid_pos.y + 1);

			for(Vei2 grid_pos = { x_start, y_start }; grid_pos.y <= y_end; ++grid_pos.y) {
				for(grid_pos.x = x_start; grid_pos.x <= x_end; ++grid_pos.x) {
					RevealTile (grid_pos);
				}
			}
		}
	}
}

MineField::Tile& MineField::TileAt (const Vei2& grid_pos) {
	return field[grid_pos.y * width + grid_pos.x];
}

const MineField::Tile& MineField::TileAt (const Vei2& grid_pos) const {
	return field[grid_pos.y * width + grid_pos.x];
}

Vei2 MineField::ScreenToGrid (const Vei2& screen_pos) {
	return (screen_pos - top_left) / SpriteCodex::tileSize;
}

int MineField::CountNeighborMines (const Vei2& grid_pos) {
	const int x_start = std::max (0, grid_pos.x - 1);
	const int y_start = std::max (0, grid_pos.y - 1);
	const int x_end = std::min (width - 1, grid_pos.x + 1);
	const int y_end = std::min (height - 1, grid_pos.y + 1);

	int count = 0;
	for(Vei2 grid_pos = { x_start, y_start }; grid_pos.y <= y_end; ++grid_pos.y) {
		for(grid_pos.x = x_start; grid_pos.x <= x_end; ++grid_pos.x) {
			if(TileAt (grid_pos).HasMine ()) {
				++count;
			}
		}
	}
	return count;
}

bool MineField::GameIsWon () const {
	for(const Tile& tile : field) {
		if((tile.HasMine () && !tile.IsFlagged ()) || (!tile.HasMine() && !tile.IsRevealed())) {
			return false;
		}
	}
	return true;
}
