def generateSortPermutations(A):
    # Step 1: Compute the sorted array B
    B = sorted(A)
    
    # Step 2: Build mapping: value -> list of indices in B
    from collections import defaultdict
    value_to_indices = defaultdict(list)
    for index, value in enumerate(B):
        value_to_indices[value].append(index)
    
    result = []  # to store all valid index permutations

    # backtracking helper
    def backtrack(i, current_perm, used):
        if i == len(A):
            result.append(current_perm.copy())
            return
        value = A[i]
        # Iterate over all candidate positions for A[i]
        for pos in value_to_indices[value]:
            if pos not in used:
                used.add(pos)
                current_perm.append(pos)
                backtrack(i + 1, current_perm, used)
                current_perm.pop()
                used.remove(pos)
    
    backtrack(0, [], set())
    return result

# Example usage:
A = [0,1,2,0,4,5,0]
permutations = generateSortPermutations(A)
print("Valid permutations (index assignments) that yield the sorted array:", permutations)