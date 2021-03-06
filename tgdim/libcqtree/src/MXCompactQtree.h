/*
 * MXCompactQtree.h
 *
 *  Created on: Apr 15, 2014
 *      Author: diegocaro
 */

#ifndef MXCOMPACTQTREE_H_
#define MXCOMPACTQTREE_H_

#include "CompactQtree.h"

#include "utils.h"
#include <cdswrapper/sdsl_bitvectors.h>

// Libcds
#include <libcdsBasics.h>
#include <BitSequenceBuilder.h>

#include <vector>

using namespace cqtree_utils;
using namespace cds_static;

namespace cqtree_static {

//const uint _K = 2;  //fixed K in K^n trees :)
struct Node {
	size_t lo, hi;
	int level;
};

class MXCompactQtree: public CompactQtree {
public:
	MXCompactQtree() {
		__setdefaultvalues();
	}

	MXCompactQtree(std::vector<Point<uint> > &points, BitSequenceBuilder *bs,
			int k1 = 2, int k2 = 2, int max_level_k1 = 0,
			int max_levels_ki = 0);

	virtual ~MXCompactQtree();

	virtual void stats_space() const;

	void all(Point<uint> p, size_t z, int level, vector<Point<uint> > &vpall);
	virtual void all(vector<Point<uint> > &vpall) {
		Point<uint> p(num_dims_);
		all(p, -1, -1, vpall);
	}

	void range(Point<uint> &p, size_t z, int level, Point<uint> &from,
			Point<uint> &to, vector<Point<uint> > &vpall, size_t &items,
			bool pushval);
	virtual size_t range(Point<uint> &from, Point<uint> &to,
			vector<Point<uint> > &vpall, bool pushval = true) {
		Point<uint> p(num_dims_);
		size_t items = 0;

		count_ops::bitmapT.clear();

		range(p, -1, -1, from, to, vpall, items, pushval);

		count_ops::bitmapT.print();

		return items;
	}

	// fast calculation of morton code, just when k1=k2=2
	inline int code_k_eq_2(const Point<uint> &p, int level) const {
		int r = 0;
		for (int i = 0; i < num_dims_; i++) {
			// r |= ((p[i]/2^(depth_-level-1)%2)^(num_dims_-i-1)
			r |= (((p[i] >> (depth_ - level - 1)) & 1) << (num_dims_ - i - 1));
		}
		return r;
	}

	// Return the morton code, considering different arity on each level of the tree
	int code(const Point<uint> &p, int level) const {
		assert(level < depth_);

		int r = 0;
		int c;

		if (false == is_interleaved_) {
			if ((levels_k1_ == 0 && k2_ == 2) || (k1_ == 2 && k2_ == 2)) {
				return code_k_eq_2(p, level);
			}

			for (int i = 0; i < num_dims_; i++) {
				c = (p[i] / nk_[level]) % k_[level];
				r += c * kpower_per_level_dim_[level * num_dims_ + i];
			}
		} else {
			//caso k1 y k2, con k2 interleaved

			for (int i = rangedim_by_level_[level].first;
					i < rangedim_by_level_[level].second; i++) {
				c = (p[i] / nk_[level]) % k_[level];
				r += c * kpower_per_level_dim_[level * num_dims_ + i];
			}
		}

		return r;
	}

	MXCompactQtree(ifstream & f);
	virtual void save(ofstream &f) const;

protected:
	void __setdefaultvalues() {
		depth_ = 0;
		max_children_ = 0;
		maxvalue_ = 0;
		levels_ki_ = 0;

		is_interleaved_ = false;

	}

	void build(std::vector<Point<uint> > &vp, BitSequenceBuilder *bs);

	size_t items_; //number of points

	int levels_k1_;
	int levels_k2_; //int virtual_depth_k2_;
	int levels_ki_; //int virtual_depth_ki_;// levels interleaved
	int k1_;
	int k2_;
	int ki_; //k used for interleaved, for now is fixed to 2

	int depth_;
	//vector<int> virtual_depth_; // depth if all three uses k1 or k2 or ki
	//int childs_;  //childs per node
	uint maxvalue_;

	int num_dims_; //number of dimensions

	vector<int> k_;
	vector<int> children_; //children per node, per level
	int max_children_;

	vector<uint> nk_; //k^d per level (o cuanto aporta cada nivel a cada dimension)

	bool is_interleaved_;

	vector<pair<int, int> > rangedim_by_level_; // range of dimensions used by level
	vector<int> dims_; //number of dimensions processed by level

	vector<size_t> nodes_by_level_; // nodes by level, or, bitmap size per level :D
	vector<BitSequence*> T_;  // tree bitmaps, one per level

	// to replace mypow and get code values
	vector<uint> kpower_per_level_dim_;
};

} /* namespace cqtree_static */

#endif /* MXCOMPACTQTREE_H_ */
