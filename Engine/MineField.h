#pragma once

#include "Graphics.h"
#include "Vei2.h"

class MineField { 
public:
	enum class State {
		lose,
		win,
		play
	};
private:
	class Tile {
	public:
		enum class State {
			hidden,
			flagged,
			revealed
		};
	public:
		void SpawnMine ();
		bool HasMine () const;
		void Draw (const Vei2& screen_pos, Graphics& gfx, MineField::State game_state) const;
		void Reveal ();
		bool IsRevealed () const;
		void ToggleFlag ();
		bool IsFlagged () const;
		void SetNeighborMineCount (int mine_count);
	private:
		State state = State::hidden;
		bool has_mine = false;
		int n_neighbor_mines = -1;
	};
public:
	MineField (const Vei2& center, int n_mines);
	void Draw (Graphics& gfx) const;
	RectI GetRect () const;
	void OnRevealClick (const Vei2& screen_pos);
	void OnFlagClick (const Vei2& screen_pos);
	State GetState () const;
private:
	Tile& TileAt (const Vei2& grid_pos);
	const Tile& TileAt (const Vei2& grid_pos) const;
	Vei2 ScreenToGrid (const Vei2& screen_pos);
	int CountNeighborMines (const Vei2& grid_pos);
	bool GameIsWon () const;
private:
	static constexpr int width = 5;
	static constexpr int height = 10;
	static constexpr int border_width = 30;
	static constexpr Color border_color = Color (192, 192, 192);

	Vei2 top_left;
	State state = State::play;
	Tile field[width * height];
};

