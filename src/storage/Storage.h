//
// Created by root on 2/21/25.
//

#ifndef DEFTRAFT_STORAGE_H
#define DEFTRAFT_STORAGE_H

#include "raftpb/raft.pb.h"
#include "raftpb/type.h"
#include <vector>
#include <memory>

namespace DeftRaft {


    class Storage {
    public:

    };

    /**
    -------------------------------------------------
    |    0     |     1     |     2     |      3     |
    | u.offset |u.offset+1 |u.offset+2 | u.offset+3 |
    -------------------------------------------------
    */
    class Unstable {
    public:
        /*
         *MaybeFirstIndex returns the index of the first possible entry in entries
         * if it has a snapshot.
         */
        DeftRaft::MaybeUInt64t MaybeFirstIndex();

        DeftRaft::MaybeUInt64t MaybeLastIndex();

        DeftRaft::MaybeUInt64t MaybeTerm(std::uint64_t index);

        Entrys NextEntries();

        DeftRaft::Snapshot *NextSnapshot();

        void AcceptInProgress();

        void StableTo(DeftRaft::EntryID id);

        void StableSnapTo(std::uint64_t index);

        void restore(const DeftRaft::Snapshot &snapshot);

        void TruncateAndAppend(Entrys ents);

        Entrys Slice(std::uint64_t lo, std::uint64_t hi);

    private:
        void ShrinkEntriesArray();

        void MustCheckOutOfBounds(std::uint64_t lo, std::uint64_t hi);

    private:
        std::unique_ptr<DeftRaft::Snapshot> m_snapshot_{};
        Entrys m_entry_{0};
        bool m_snapshot_in_progress_{false};
        std::uint64_t m_offset_in_progress_{0};//[m_offset_in_progress_:]is wait for progress
        std::uint64_t m_offset_{0};
    };

} // CLSN

#endif //DEFTRAFT_STORAGE_H
