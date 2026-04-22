

#ifndef LLVM_FUZZER_MUTATE_H
#define LLVM_FUZZER_MUTATE_H

#include "FuzzerDefs.h"
#include "FuzzerDictionary.h"
#include "FuzzerRandom.h"

namespace fuzzer {

class MutationDispatcher {
public:
  MutationDispatcher(Random &Rand, const FuzzingOptions &Options);
  ~MutationDispatcher() {}

  void StartMutationSequence();

  void PrintMutationSequence();

  void RecordSuccessfulMutationSequence();

  size_t Mutate_Custom(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_CustomCrossOver(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_ShuffleBytes(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_EraseBytes(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_InsertByte(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_InsertRepeatedBytes(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_ChangeByte(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_ChangeBit(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_CopyPart(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_AddWordFromManualDictionary(uint8_t *Data, size_t Size,
                                            size_t MaxSize);

  size_t Mutate_AddWordFromTemporaryAutoDictionary(uint8_t *Data, size_t Size,
                                                   size_t MaxSize);

  size_t Mutate_AddWordFromTORC(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_AddWordFromPersistentAutoDictionary(uint8_t *Data, size_t Size,
                                                    size_t MaxSize);

  size_t Mutate_ChangeASCIIInteger(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_ChangeBinaryInteger(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate_CrossOver(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t Mutate(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t DefaultMutate(uint8_t *Data, size_t Size, size_t MaxSize);

  size_t CrossOver(const uint8_t *Data1, size_t Size1, const uint8_t *Data2,
                   size_t Size2, uint8_t *Out, size_t MaxOutSize);

  void AddWordToManualDictionary(const Word &W);

  void AddWordToAutoDictionary(DictionaryEntry DE);
  void ClearAutoDictionary();
  void PrintRecommendedDictionary();

  void SetCorpus(const InputCorpus *Corpus) { this->Corpus = Corpus; }

  Random &GetRand() { return Rand; }

private:

  struct Mutator {
    size_t (MutationDispatcher::*Fn)(uint8_t *Data, size_t Size, size_t Max);
    const char *Name;
  };

  size_t AddWordFromDictionary(Dictionary &D, uint8_t *Data, size_t Size,
                               size_t MaxSize);
  size_t MutateImpl(uint8_t *Data, size_t Size, size_t MaxSize,
                    const std::vector<Mutator> &Mutators);

  size_t InsertPartOf(const uint8_t *From, size_t FromSize, uint8_t *To,
                      size_t ToSize, size_t MaxToSize);
  size_t CopyPartOf(const uint8_t *From, size_t FromSize, uint8_t *To,
                    size_t ToSize);
  size_t ApplyDictionaryEntry(uint8_t *Data, size_t Size, size_t MaxSize,
                              DictionaryEntry &DE);

  template <class T>
  DictionaryEntry MakeDictionaryEntryFromCMP(T Arg1, T Arg2,
                                             const uint8_t *Data, size_t Size);

  Random &Rand;
  const FuzzingOptions &Options;

  Dictionary ManualDictionary;

  Dictionary TempAutoDictionary;

  Dictionary PersistentAutoDictionary;

  std::vector<Mutator> CurrentMutatorSequence;
  std::vector<DictionaryEntry *> CurrentDictionaryEntrySequence;

  static const size_t kCmpDictionaryEntriesDequeSize = 16;
  DictionaryEntry CmpDictionaryEntriesDeque[kCmpDictionaryEntriesDequeSize];
  size_t CmpDictionaryEntriesDequeIdx = 0;

  const InputCorpus *Corpus = nullptr;
  std::vector<uint8_t> MutateInPlaceHere;

  std::vector<Mutator> Mutators;
  std::vector<Mutator> DefaultMutators;
};

}

#endif
