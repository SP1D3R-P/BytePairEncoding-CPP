
try :
    import _core
except : 
    raise ImportError("_core is not found. Compile the cpp files to get the library")

from typing import Callable , Any , List , Dict , Optional
from typing_extensions import Self
import beartype


@beartype.beartype
class BytePairEncoding(_core.pyBytePairEncoding) : 

    _special_tokens : dict[str,int] = {
        'a' : 3,
        'b' : 3
    }

    def __init__(self : Self,
                vocab_size : Optional[int]  = 0x100 ,
                pattern : Optional[str] = 'GPT4' ,
                special_tokens : Optional[str | set[str]]  = 'all'):
        """BytePairEncoding

        Args:
            self (Self): self
            vocab_size (int, optional): vocabulary size . Defaults to 0x100.
            pattern (str, optional): pattern to use [can be of given option or valid regex pattern]. Defaults to 'GPT4'.
            special_tokens (str | set[str], optional): valid special token ['all',{'the set of token you want'}]. Defaults to 'all'.
        """
        assert vocab_size >= 0x100 and "vocab size must be greater or equall to 256"
        if not pattern in ('GPT4','CLK100'):
            super().__init__(vocab_size,'custom_pattern',pattern)
        else : 
            super().__init__(vocab_size)


    @property
    def vocab_size(self) -> int :
        """gets the vocab size of the tokenizer

        Returns:
            int: vocab size 
        """
        return super().getVocabSize()

    @property
    def pattern(self) -> str :
        """currently using pattern

        Returns:
            str: pattern
        """
        return super().getPatternUsed()

    def train(self : Self) -> None:
        """train the tokenizer based on the Encoding 
        """
        super().train()
     

    @beartype.beartype
    def compile(self, f_name : str ) -> None :
        """compiles the file string to vector for further processing 

        Args:
            f_name (str): file name 
        """
        with open(f_name,'r') as fp : 
            super().compile(fp.read(),f_name)



    @beartype.beartype
    def save(self,f_name : str ) -> None : 
        """saves the model 

        Args:
            f_name (str): file name 
        """
        super().save()

    

    

    

    