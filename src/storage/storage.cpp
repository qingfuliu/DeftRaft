#include <memory>

#include "Storage.h"

namespace DeftRaft {

    DeftRaft::MaybeUInt64t Unstable::MaybeFirstIndex() {
        if (nullptr == m_snapshot_) {
            return {false, 0};
        }
        return {true, m_snapshot_->metadata().index() + 1};
    }

    DeftRaft::MaybeUInt64t Unstable::MaybeLastIndex() {
        if (!m_entry_.empty()) {
            return {true, m_entry_.size() + m_offset_ - 1};
        }
        if (nullptr != m_snapshot_) {
            return {true, m_snapshot_->metadata().index()};
        }
        return {false, 0};
    }

    DeftRaft::MaybeUInt64t Unstable::MaybeTerm(std::uint64_t index) {
        if (index < m_offset_) {
            if (nullptr != m_snapshot_ && m_snapshot_->metadata().index() == index) {
                return {true, m_snapshot_->metadata().term()};
            }
        }

        if (auto last = MaybeLastIndex();last.nonnull && index <= last.value) {
            return {true, m_entry_[index - m_offset_].term()};
        }

        return {false, 0};
    }

    Entrys Unstable::NextEntries() {
        auto inProgress = m_offset_in_progress_ - m_offset_;
        if (inProgress == m_entry_.size()) {
            return {};
        }
        return Entrys{m_entry_.begin() + inProgress, m_entry_.end()};
    }

    DeftRaft::Snapshot *Unstable::NextSnapshot() {
        if (nullptr == m_snapshot_ || m_snapshot_in_progress_) {
            return nullptr;
        }
        return m_snapshot_.get();
    }

    void Unstable::AcceptInProgress() {
        if (!m_entry_.empty()) {
            m_offset_in_progress_ = m_entry_.back().index() + 1;
        }
        if (nullptr != m_snapshot_) {
            m_snapshot_in_progress_ = true;
        }
    }

    void Unstable::StableTo(DeftRaft::EntryID id) {
        auto term = MaybeTerm(id.m_index_);
        if (!term.nonnull) {
            return;
        }
        //get term from snapshot
        if (id.m_index_ < m_offset_) {
            return;
        }
        //conflict
        if (id.m_term_ != term.value) {
            return;
        }
        m_entry_ = Entrys{m_entry_.begin() + id.m_index_ - m_offset_ + 1, m_entry_.end()};
        m_offset_ = id.m_index_ + 1;
        m_offset_in_progress_ = std::max(m_offset_, m_offset_in_progress_);
        ShrinkEntriesArray();
    }

    void Unstable::StableSnapTo(std::uint64_t index) {
        if (nullptr == m_snapshot_) {
            return;
        }
        if (m_snapshot_->metadata().index() != index) {
            return;
        }
        m_snapshot_in_progress_ = false;
        m_snapshot_.reset(nullptr);

    }

    void Unstable::restore(const DeftRaft::Snapshot &snapshot) {
        m_offset_ = snapshot.metadata().index() + 1;
        m_offset_in_progress_ = m_offset_;
        m_entry_.clear();
        m_snapshot_ = std::make_unique<DeftRaft::Snapshot>(snapshot);
        m_snapshot_in_progress_ = false;
    }

    void Unstable::TruncateAndAppend(Entrys ents) {
        if (ents.empty()) {
            return;
        }
        auto fromIndex = ents[0].index();
        if (fromIndex == m_offset_ + m_entry_.size()) {
            m_entry_.reserve(m_entry_.size() + ents.size());
            m_entry_.insert(m_entry_.end(), std::move_iterator(ents.begin()), std::move_iterator(ents.end()));
        } else if (fromIndex <= m_offset_) {
            m_entry_ = std::move(ents);
            m_offset_ = fromIndex;
            m_offset_in_progress_ = fromIndex;
        } else {
            auto keep = fromIndex - m_offset_; //[0:keep)
            m_entry_.erase(m_entry_.begin() + keep, m_entry_.end());
            m_entry_.insert(m_entry_.end(), std::move_iterator(ents.begin()), std::move_iterator(ents.end()));
            m_offset_in_progress_ = std::min(m_offset_in_progress_, m_entry_.back().index());
        }

    }

    Entrys Unstable::Slice(std::uint64_t lo, std::uint64_t hi) {
        MustCheckOutOfBounds(lo, hi);
        return {m_entry_.begin() + lo, m_entry_.begin() + hi};
    }

    void Unstable::ShrinkEntriesArray() {
        if ((m_entry_.size() << 1) < m_entry_.capacity()) {
            m_entry_.shrink_to_fit();
        }
    }

    void Unstable::MustCheckOutOfBounds(std::uint64_t lo, std::uint64_t hi) {
        if (lo > hi) {
            throw std::runtime_error("check failed");
        }
        auto upper = m_offset_ + m_entry_.size();
        if (lo < m_offset_ || hi > upper) {
            throw std::runtime_error("check failed");
        }
    }

}